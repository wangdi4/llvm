// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include <Compiler.h> // LLVM_FALLTHROUGH
#include "exceptions.h" // LLVM_BACKEND_UNUSED
#include "opencl_printf_ext.h"
#include <cstdarg> // va_list
#include <assert.h>
#include <climits> // INT_MAX

#if defined(_WIN32)
    #include <process.h>
    #include <stdint.h>
#endif

using namespace std;
using namespace Intel::OpenCL;


// Maximal length of a single format specifier (taken with a large spare)
//
const size_t MAX_FORMAT_LEN = 128;

// Maximal length produced by a single conversion. Note that C99 dictates in
// 7.19.6.1 that this must be minimum 4095 to conform to the standard.
//
// TODO: changed to 1024 as a WORKAROUND to memory corruption / stack overflow by opencl_printf
const size_t MAX_CONVERSION_LEN = 1024; // <- was 4096;


// These macros allow working with the packed args buffer similarly to the
// way the standard va_* macros work. Note that the minimal size of argument
// taken off the buffer is sizeof(int) - this is guaranteed by the default
// argument promotion rules of C99.
//
#define INTSIZEOF(n) ((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))

template <typename T>
const char* CopyAndAdvance(const char* src, T& dest) {
    std::copy(src, src + sizeof(T), (char*)&dest);
    return src + INTSIZEOF(T);
}


// An accumulator that writes into a FILE* stream.
//
StreamOutputAccumulator::StreamOutputAccumulator(FILE* stream)
    : stream(stream), count(0)
{
    #if (defined(_WIN32) || defined(_WIN64))
        hStdout=nullptr;
    #endif
}

void StreamOutputAccumulator::append(char c)
{
#if (defined(_WIN32) || defined(_WIN64))
    //Windows 64 crash fix, ticket no. CSSD100006413
    if (!hStdout){
        hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        if (INVALID_HANDLE_VALUE == hStdout){
            hStdout = nullptr;
            return;
        }
    }
    DWORD d;
    WriteFile(hStdout, &c, 1, &d, nullptr);
#else
    fputc(c, stream);
#endif
    count++;
}

void StreamOutputAccumulator::finalize()
{
}

int StreamOutputAccumulator::output_count()
{
    return count;
}



// An accumulator that writes into a size-limited buffer. No more than size
// chars will ever be written into the buffer. finalize() inserts the final
// '\0'.
//
StringOutputAccumulator::StringOutputAccumulator(char* outstr_, size_t outstr_size_)
    : outstr(outstr_), outstr_size(outstr_size_), outstr_ptr(0), count(0)
{}

void StringOutputAccumulator::append(char c)
{
    // Stop writing before the last cell in the buffer
    //
    if (outstr_ptr < outstr_size - 1)
        outstr[outstr_ptr++] = c;
    count++;
}

void StringOutputAccumulator::finalize()
{
    assert(outstr_ptr < outstr_size);
    outstr[outstr_ptr] = '\0';
}

int StringOutputAccumulator::output_count()
{
    // C99 says in "7.19.6.5 The snprintf function":
    //
    //  The snprintf function returns the number of characters that
    //  would have been written had n been sufficiently large, not counting
    //  the terminating null character, or a negative value if an encoding
    //  error occurred. Thus, the null-terminated output has been
    //  completely written if and only if the returned value is nonnegative
    //  and less than n.
    //
    return count;
}

// Conversion flags.
//
enum ConversionFlag {
    FLAG_NONE       = 0x00,
    FLAG_MINUS      = 0x01,
    FLAG_PLUS       = 0x02,
    FLAG_SPACE      = 0x04,
    FLAG_POUND      = 0x08,
    FLAG_ZERO       = 0x10
};


// Length modifier type.
//
enum LengthModifier {
    MODIFIER_NONE,
    MODIFIER_CHAR,
    MODIFIER_SHORT,
    MODIFIER_LONG,
    MODIFIER_LONGLONG,
    MODIFIER_INTMAX,
    MODIFIER_SIZE_T,
    MODIFIER_PTRDIFF,
    MODIFIER_LONGDOUBLE,
    //opencl 1.2 specific modifier, which suggests 32 bit conversion for vector elements
    MODIFIER_VEC32BITELEMENT
};


// Turn a digit character ('1', '2', etc.) into its integer meaning (1, 2...)
//
inline int CHARDIGIT(char digit)
{
    return digit - '0';
}


// A C99 conformant wrapper around Microsoft's non-C99-compliant functions.
// The thing this wrapper fixes are:
// * Correct handling of a NULL str in case size = 0
// * Always '\0'-terminate str, and if it's too short return the amount of
//   chars that would've been written into it, instead of returning -1.
//
// For Linux/gcc, this should be made a trivial wrapper around vsnprintf.
//
int c99_snprintf(char* str, size_t size, const char* format, ...)
{
    size_t count;
    va_list ap;
    va_start(ap, format);
#if defined(_WIN32)
    count = _vscprintf(format, ap);
#else
    char test_buf[1];
    count = vsnprintf( test_buf, sizeof(test_buf), format, ap);
#endif
    va_end(ap);
    if (!str)
        return count;
    va_start(ap, format);
#if defined(_WIN32)
    _vsnprintf_s(str, size, _TRUNCATE, format, ap);
#else
    vsnprintf(str, size, format, ap);
#endif
    va_end(ap);
    return count;
}


// This is required with MSVC to make the %e format behave the way required
// by C99.
// XXX: When porting, take care...
//
void c99_fix_format_e()
{
  // starting VS2015 two-digit exponent mode is now the default and the non-conforming,
  // previously-default, three-digit exponent format is no longer supported.
#if defined(_MSC_VER) && _MSC_VER < 1900
    _set_output_format(_TWO_DIGIT_EXPONENT);
#endif
}


// Construct in buf a format string for snprintf.
// If buf is too short, return -1 (it should happen only in case of a bug!).
// Otherwise, return 0.
//
int build_format_string(char* buf, int buflen,
                        unsigned conversion_flags,
                        int width, int precision,
                        LengthModifier modifier,
                        char conversion_type)
{
    // Buffer to hold flags: there are only 5 types of flags
    //
    char flags_buf[6] = {0};
    char* ptr = flags_buf;
    if (conversion_flags & FLAG_MINUS) *ptr++ = '-';
    if (conversion_flags & FLAG_PLUS) *ptr++ = '+';
    if (conversion_flags & FLAG_SPACE) *ptr++ = ' ';
    if (conversion_flags & FLAG_POUND) *ptr++ = '#';
    if (conversion_flags & FLAG_ZERO) *ptr++ = '0';

    int count;

    // Buffer to hold width: enough to fit the longest 64-bit integer with
    // a sign.
    //
    char width_buf[25] = {0};
    if (width != 0) {
        count = c99_snprintf(width_buf, sizeof(width_buf), "%d", width);
        if (size_t(count) >= sizeof(width_buf))
            return -1;
    }

    char precision_buf[25] = {0};
    char dot_buf[2] = {0};
    if (precision >= 0) {
        dot_buf[0] = '.';
        count = c99_snprintf(precision_buf, sizeof(precision_buf), "%d", precision);
        if (size_t(count) >= sizeof(precision_buf))
            return -1;
    }

    // Buffer to hold length modifier: the longest is 2 chars
    //
    char modifier_buf[3] = {0};
    switch (modifier) {
        case MODIFIER_CHAR:
            modifier_buf[0] = 'h';
            modifier_buf[1] = 'h';
            break;
        case MODIFIER_SHORT:
            modifier_buf[0] = 'h';
            break;
        case MODIFIER_LONG:
            modifier_buf[0] = 'l';
#if defined (_WIN32) || (_WIN64)
            //windows formaating is buggy, since %l shows 32bit only
            modifier_buf[1] = 'l';
#endif
            break;
        case MODIFIER_LONGLONG:
            modifier_buf[0] = 'l';
            modifier_buf[1] = 'l';
            break;
        case MODIFIER_INTMAX:
            modifier_buf[0] = 'j';
            break;
        case MODIFIER_SIZE_T:
            //"%z" is not supported by C99 snprintf,
            //therefore supplementing with "%(empty)" (leaving "%u" or "%d") in 32-bit "%ll" in 64-bit
            //modifier_buf[0] = 'z';
            if (sizeof(size_t)==8){
                modifier_buf[0] = 'l';
                modifier_buf[1] = 'l';
            }
            break;
        case MODIFIER_PTRDIFF:
            modifier_buf[0] = 't';
            break;
        case MODIFIER_LONGDOUBLE:
            modifier_buf[0] = 'L';
            break;
        default:
            break;
    }

    count = c99_snprintf(buf, buflen, "%%%s%s%s%s%s%c",
        flags_buf, width_buf,
        dot_buf, precision_buf,
        modifier_buf,
        conversion_type);

    if (count >= buflen) {
        assert(0);
        return -1;
    }
    else
        return 0;
}


inline bool is_unsigned_specifier(char c)
{
    return (c == 'X' || c == 'x' || c == 'o' || c == 'u');
}

#if defined(_WIN32) || defined(_WIN64)
//union for double bit operations
static const __int64 MASK = 0x8000000000000000;
static const __int64 dnan_min =  0x7FF0000000000000; // or numeric_limits<float>::quiet_NaN();
static const __int64 dnan_max = 0x7FFFFFFFFFFFFFFF;  // or numeric_limits<float>::quiet_NaN();
union DoubleUtil{
    double m_d;
    __int64 m_i;

    DoubleUtil(double d): m_d(d){}
    bool isNan()const{ return !( m_d==m_d ); }
    bool isInf()const{ return (m_i & dnan_max) == dnan_min; }
    bool isNegative()const {return MASK == (m_i & MASK);}
};
#endif //WIN

//Purpose: appends a string to the output buffer, with respect to the value of
//the given double value (d).
static void purgeDouble(OutputAccumulator& output, const double& d,
                        char* buffer, bool capitalCase) {
//window's sprintf does not conform with OpenCL when it comes to NAN and INF values
#if defined(_WIN32) || defined(_WIN64)
    DoubleUtil dutil(d);
    if (dutil.isNegative() && (dutil.isInf() || dutil.isNan()))
        output.append('-');
    if (dutil.isNan()) {
        output.append(capitalCase ? "NAN" : "nan");
        return;
    }
    if (dutil.isInf()) {
        output.append(capitalCase ? "INF" : "inf");
        return;
    }
#endif //WIN
    output.append(buffer);
}
// Does the heavy lifting of formatted printing - called by the interface
// functions.
// Return 0 if everything is OK and a negative value in case of an error.
//
// Terminology follows the C99 standard (section 7.19.6):
//
//    A "conversion specification" is a '%' character followed by flags, width,
//    precision, length and specifier (all optional except the last).
//
//
static int formatted_output(OutputAccumulator& output, const char* format, const char* args)
{
    // Reject NULL format
    //
    if (!format)
        return -1;

    c99_fix_format_e();

    // c is the current character in the format string
    //
    char c = *format;

    // Main loop that goes over entities in the format string. An entity is
    // either a conversion specification or a sequence of non-% chars.
    //
    while (c != '\0') {
        if (c != '%') {
            // This is not a conversion. Just output all characters until a %
            // or end of format string is encountered.
            //
            while (c != '%' && c != '\0') {
                output.append(c);
                c = *++format;
            }

            // End of format string, we're done
            //
            if (c == '\0')
                break;
        }

        // Here we have a % char in c. Skip it and parse the conversion
        // specification
        //
        c = *++format;

        if (c == '\0') {
            break;
        }

        // Parse flags
        //
        unsigned conversion_flags = FLAG_NONE;
        bool endloop = false;
        do {
            switch (c) {
                case '-': conversion_flags |= FLAG_MINUS;    break;
                case '+': conversion_flags |= FLAG_PLUS;     break;
                case ' ': conversion_flags |= FLAG_SPACE;    break;
                case '#': conversion_flags |= FLAG_POUND;    break;
                case '0': conversion_flags |= FLAG_ZERO;     break;
                default: endloop = true;                    break;
            }
        } while (!endloop && ((c = (*++format)) != '\0'));

        // Parse width
        int width = 0;
        if (c == '*') {
            // Note: according to C99, a negative width argument means a '-'
            // flag followed by a positive width.
            //
            args = CopyAndAdvance(args, width);
            if (width < 0) {
                width = -width;
                conversion_flags |= FLAG_MINUS;
            }
            c = *++format;
        }
        else {
            while (c >= '0' && c <= '9') {
                width = width * 10 + CHARDIGIT(c);
                if (width >= INT_MAX / 10)  // overflow
                    return -1;
                c = *++format;
            }
        }

        // Parse precision.
        // Note: according to C99, a negative precision means no precision
        //
        int precision = 0;
        if (c == '.') {
            c = *++format;    // skip the dot
            if (c == '*') {
                args = CopyAndAdvance(args, precision);
                c = *++format;
            }
            else {
                while (c >= '0' && c <= '9') {
                    precision = precision * 10 + CHARDIGIT(c);
                    if (precision >= INT_MAX / 10)  // overflow
                        return -1;
                    c = *++format;
                }
            }
        }
        else {
            precision = -1;
        }

        // Parse vector modifier (OpenCL specific).
        // vector_len = 1 means no vector modifier. Otherwise, it specifies
        // the vector length requested.
        //
        unsigned vector_len = 1;
        if (c == 'v') {
            // the number after 'v' can consist of one or two digits
            //
            c = *++format;
            if (isdigit(c)) {
                vector_len = c - '0';
                c = *++format;

                if (isdigit(c)) {
                    vector_len = vector_len * 10 + (c - '0');
                    c = *++format;
                }
            }
        }

        // Parse length modifier.
        //
        LengthModifier modifier = MODIFIER_NONE;
        switch (c) {
            case 'h':
                modifier = MODIFIER_SHORT;
                c = *++format;
                if (c == 'h') {
                    modifier = MODIFIER_CHAR;
                    c = *++format;
                } else if (c == 'l'){
                    //hl vector conversion modifier (see opencl spec p. 287)
                    modifier = MODIFIER_VEC32BITELEMENT;
                    c = *++format;
                }
                break;
            case 'l':
                modifier = MODIFIER_LONG;
                c = *++format;
                if (c == 'l') {
                    modifier = MODIFIER_LONGLONG;
                    c = *++format;
                }
                break;
            case 'j':
                modifier = MODIFIER_INTMAX;
                c = *++format;
                break;
            case 'L':
                modifier = MODIFIER_LONGDOUBLE;
                c = *++format;
                break;
            case 'z':
                modifier = MODIFIER_SIZE_T;
                c = *++format;
                break;
            case 't':
                modifier = MODIFIER_PTRDIFF;
                c = *++format;
                break;
            default:
                break;
        }
        // At this point the following variables specify all the state parsed
        // prior to this specifier:
        //   unsigned conversion_flags
        //   int width
        //   int precision
        //   LengthModifier modifier
        //   unsigned vector_len
        //
        // We can now build the format string back in order to pass it to
        // c99_snprintf. This is a scalar format string (for vectors, the
        // vector length specifier is removed).
        //
        char format_buf[MAX_FORMAT_LEN] = {0};
        int rc = build_format_string(format_buf, sizeof(format_buf),
            conversion_flags, width, precision, modifier, c);

        if (rc < 0)
            return rc;

        // This is the conversion buffer
        //
        char cbuf[MAX_CONVERSION_LEN];
        size_t cbuflen = sizeof(cbuf);

        // Now that we have the format string in format_buf, we will take an
        // argument of the appropriate type off the stack and pass it for
        // conversion to c99_snprintf.
        //
        // The following are temp vars for taking arguments from the args list
        //
        char char_val;
        const char* str_val;
        uint64_t int_val;
        double float_val;
        char charbuf[2] LLVM_BACKEND_UNUSED = {0};
        void* voidptr = nullptr;
        bool capitalCase = false;

        // Parse the conversion specifier and perform the actual conversion.
        // Note that some of the conversions manually manipulate the "args"
        // pointer, since vector types have to be supported, with their
        // special alignment and spacing rules on the stack. So, we assume
        // here that varargs are passed according to the Microsoft __cdecl
        // calling convention.
        //
        switch (c) {
            case '%':
                output.append('%');
                break;
            case 'c':
                for (unsigned i = 0; i < vector_len; ++i) {
                    args = CopyAndAdvance(args, char_val);

                    if (size_t(c99_snprintf(cbuf, cbuflen, format_buf, char_val)) >= cbuflen)
                        return -1;
                    output.append(cbuf);

                    if (i < vector_len - 1)
                        output.append(',');
                }
                break;
            case 's':
              if (nullptr != *(const char**)args) {
                  args = CopyAndAdvance(args, str_val);
                }
                else{
                  //a 'fatalic' case, in which the string pointer is NULL.
                  //We print the null string, and advancing the buffer pointer
                  //by 4 bytes.
                  str_val = "(null)";
                  args += sizeof(int);
                }
                if (size_t(c99_snprintf(cbuf, cbuflen, format_buf, str_val)) >= cbuflen)
                    return -1;
                output.append(cbuf);
                break;
            case 'p':
                args = CopyAndAdvance(args, voidptr);

                if (size_t(c99_snprintf(cbuf, cbuflen, format_buf, voidptr)) >= cbuflen)
                    return -1;
                output.append(cbuf);
                break;
            case 'd': // fall-through
            case 'i':
            case 'X':
            case 'x':
            case 'o':
            case 'u':
                for (unsigned i = 0; i < vector_len; ++i) {
                    switch (modifier) {
                        case MODIFIER_CHAR:
                            if (is_unsigned_specifier(c)){
                                cl_uchar dest;
                                args = CopyAndAdvance(args, dest);
                                int_val = dest;
                            }
                            else{
                                cl_char dest;
                                args = CopyAndAdvance(args, dest);
                                int_val = dest;
                            }
                            break;
                        case MODIFIER_SHORT:
                            if (is_unsigned_specifier(c)){
                                cl_ushort dest;
                                args = CopyAndAdvance(args, dest);
                                int_val = dest;
                            }
                            else{
                                cl_short dest;
                                args = CopyAndAdvance(args, dest);
                                int_val = dest;
                            }
                            break;
                        case MODIFIER_LONG:
                        case MODIFIER_LONGLONG:
                        case MODIFIER_INTMAX:
                            // The OpenCL printf extension proposal requires
                            // 64-bit integers with the 'l' modifier, so
                            // (u)int64_t is taken. intmax and longlong aren't
                            // supported, so we keep them as 'long'.
                            //
                            if (is_unsigned_specifier(c)){
                                uint64_t dest;
                                args = CopyAndAdvance(args, dest);
                                int_val = dest;
                            }
                            else{
                                int64_t dest;
                                args = CopyAndAdvance(args, dest);
                                int_val = dest;
                            }
                            break;
                        case MODIFIER_PTRDIFF:
                            {
                            ptrdiff_t dest;
                            args = CopyAndAdvance(args, dest);
                            int_val = dest;
                            }
                            break;
                        case MODIFIER_SIZE_T:
                            {
                            size_t dest;
                            args = CopyAndAdvance(args, dest);
                            int_val = dest;
                            }
                            break;
                        case MODIFIER_VEC32BITELEMENT: //fall through
                        default:
                            if (is_unsigned_specifier(c)){
                                cl_uint dest;
                                args = CopyAndAdvance(args, dest);
                                int_val = dest;
                            }
                            else{
                                cl_int dest;
                                args = CopyAndAdvance(args, dest);
                                int_val = dest;
                            }
                            break;
                    }

                    if (size_t(c99_snprintf(cbuf, cbuflen, format_buf, int_val)) >= cbuflen)
                        return -1;
                    output.append(cbuf);

                    if (i < vector_len - 1)
                        output.append(',');
                }
                break;
                // case 'n':// XXX: Should this be supported in OpenCL at all??
                            // it's unsafe and isn't supported by MSVC
            case 'F': // fall-through
            case 'E':
            case 'G':
            case 'A':
              capitalCase = true;
              LLVM_FALLTHROUGH;
            case 'f':
            case 'e':
            case 'g':
            case 'a':
                for (unsigned i = 0; i < vector_len; ++i) {
                    // Practically, the LONGDOUBLE modifier is ignored since OpenCL
                    // has no such type.
                    //
                    args = CopyAndAdvance(args, float_val);

                    if (size_t(c99_snprintf(cbuf, cbuflen, format_buf, float_val)) >= cbuflen)
                        return -1;
                    //in some cases (i.e., inf, nan), values, we need to reformat
                    //the string representation of double values
                    purgeDouble(output, float_val, cbuf, capitalCase);
                    if (i < vector_len - 1)
                        output.append(',');
                }
                break;
            default:
                cbuf[0] = '\0';
                break;
        }

        // Next character
        //
        c = *++format;
    }

    return 0;
}

int printFormatCommon(OutputAccumulator& output, const char* format, const char* args){
    int rc = formatted_output(output, format, args);
    output.finalize();
    return rc < 0 ? rc : output.output_count();
}

extern "C" LLVM_BACKEND_API int opencl_snprintf(char* outstr, size_t size, const char* format, char* args, void* pCallback, void* pHandle)
{
    StringOutputAccumulator output(outstr, size);
    return printFormatCommon(output, format, args);
}
