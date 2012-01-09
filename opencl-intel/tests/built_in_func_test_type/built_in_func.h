#pragma once

#define PROCESSED_OK    0
#define PROCESSED_FAIL  1
#define PROCESSED_NONE  -1

enum tTestStatus {
	TEST_STARTED,
	TEST_FAILED,
	TEST_SUCCESSED,
	TEST_UNTOUCHED
};

class Discrete
{
public:
	bool            m_bProcessed;
	bool            m_bList;
	bool            m_bDone;
	int             m_Succeeded;
	int             m_Failed;
	char*           m_Chapter;
	char*           m_Function;
	char*           m_Tail;
	// each test will change the following data to make info available accross exceptions
	const char*     m_TestChapter;
	const char*     m_TestFunction;
	const char*     m_TestTail;
	Discrete(void);
	~Discrete(void);
} ;
typedef class Discrete tDiscrete;

const char          TESTER[] = "BuildInFunc_test_type.exe";

class CParseString
{
private:
	char*			m_str;
	char*			m_first_semicolon;
	char*			m_last_semicolon;

public:
					CParseString(const char* str);
					~CParseString();
	char*			GetChapter();
	char*			GetFunction();
	char*			GetTail();
};

class CBuildInFunc
{
private:
	static tDiscrete*      m_pDiscrete;

public:
						CBuildInFunc(tDiscrete* pDiscr);
						~CBuildInFunc();
	static void			EnableDump(void);
	static void			SetProcessed(void);
	static void			ResetStatistic(void);

	static void			SetChapter(char* cpt);
	static void			SetFunction(char* func);
	static void			SetTail(char* tail);
	static char*		GetChapter(void);
	static char*		GetFunction(void);
	static char*		GetTail(void);

	static void			SetTestChapter(const char* cpt);
	static void			SetTestFunction(const char* func);
	static void			SetTestTail(const char* tail);
	static const char*	GetTestChapter(void);
	static const char*	GetTestFunction(void);
	static const char*	GetTestTail(void);

	static bool			IsProcessed(void);
	static bool			IsDump(void);
	static bool			IsAllFunc(void);
	static bool			IsDone(void);

	static void			SetDone(void);
	static void			SetException(void);

	static bool			Start(tTestStatus* status, const char* test, const char* func, const char* tail);        // Returns false for continue, true for exit
	static int			Finish(tTestStatus* status, const char* test, const char* func, const char* tail, const int rc);

	static int			GetSucceeded();
	static int			GetFailed();
};
