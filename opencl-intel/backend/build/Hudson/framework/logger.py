'''
Defines the default logger 
'''
import sys,re
# Try to import the colorama 
try:
    from colorama import Fore, Back, Style, init, deinit
except Exception:
    from color import Fore, Back, Style, init, deinit

class STYLE:
    NORMAL = Style.RESET_ALL
    ERROR  = Fore.RED + Style.BRIGHT
    PASS   = Fore.GREEN
    WARN   = Fore.YELLOW
    INFO   = Fore.CYAN
    HIGH   = Style.BRIGHT
    
    def __init__(self):
        pass

class Logger:
    """
    Simple text logger with optional support for escaped color output.
    This logger will automatically support Colorama library if such presents 
    in the system
    """
    ANSI_RE = re.compile('\033\[((?:\d|;)*)([a-zA-Z])')
    
    def __init__(self):
        self.colorEnabled = False
    
    def enableColor(self, enable):
        """
        enables / disables the color output. 
        in disabled mode (default) all the colore escape sequences will be
        removed
        """
        if not self.colorEnabled and enable:
            init()
        elif self.colorEnabled and not enable:
            deinit()
        self.colorEnabled = enable

    def write_plain(self, stream, text, start, end):
        """
        Internal method. 
        Writes the given part of the text to supplied stream as is
        """
        if start < end:
            stream.write(text[start:end])
    
    def write_stripped(self, stream, text):
        """
        Writes the given text to the supplied stream, removing all 
        the color escape sequences
        """
        cursor = 0
        for match in self.ANSI_RE.finditer(text):
            start, end = match.span()
            self.write_plain(stream, text, cursor, start)
            cursor = end
        self.write_plain(stream, text, cursor, len(text))
        stream.flush()

    def write(self, stream, text):
        """
        Writes the given text to the stream, removing all the color
        escape sequences in case the color was disabled
        """
        if self.colorEnabled:
            stream.write(text)
            stream.flush()
        else:
            self.write_stripped(stream, text)

    def prints(self, text = None):
        """
        Writes the given text to the stream, removing all the color
        escape sequences in case the color was disabled
        """
        text = '' if text is None else text
        
        self.write(sys.stdout, text + '\n')


"""
Globally defined logger
"""
gLog = Logger()
