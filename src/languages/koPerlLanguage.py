#!python
# Copyright (c) 2000-2006 ActiveState Software Inc.
# See the file LICENSE.txt for licensing information.

"""Perl-specific Language Services implementations."""

import os, sys
from xpcom import components, ServerException
from koLanguageServiceBase import *
import logging
import pprint
import re


#---- globals

log = logging.getLogger("koPerlLanguage")
#log.setLevel(logging.DEBUG)

sci_constants = components.interfaces.ISciMoz



#---- internal support routines

def isident(char):
    return "a" <= char <= "z" or "A" <= char <= "Z" or char == "_"

def isdigit(char):
    return "0" <= char <= "9"


#---- Language Service component implementations

class KoPerlLanguage(KoLanguageBase):
    name = "Perl"
    _reg_desc_ = "%s Language" % name
    _reg_contractid_ = "@activestate.com/koLanguage?language=%s;1" \
                       % (name)
    _reg_clsid_ = "{911E8F76-C8F9-46f2-A930-1F1693400FCB}"
    _reg_categories_ = [("komodo-language", name)]

    modeNames = ['perl']
    shebangPatterns = [re.compile(ur'\A#!.*perl.*$', re.IGNORECASE | re.MULTILINE)]
    primary = 1
    internal = 0
    accessKey = 'p'

    defaultExtension = ".pl"
    # XXX read url from some config file
    downloadURL = 'http://www.ActiveState.com/Products/ActivePerl'
    commentDelimiterInfo = { "line": [ "#" ]  }
    variableIndicators = '$@%'
    namedBlockRE = "^[ \t]*?(sub\s+\w+|package\s+\w)"
    namedBlockDescription = 'Perl subs and packages'

    _lineup_chars = u"{}()[]"
    _lineup_open_chars = "([{" # Perl tells the difference between the indent and lineup {}'s
    _lineup_close_chars = ")]}"

    supportsSmartIndent = "brace"
    importPref = "perlExtraPaths"
    sample = r"""#  Fruit salad recipe
my %salad;
while (<DATA>) {
	$salad{$1} = $2 if /^([a-z]+)\s+(\d+)/;
}
my @fruits = keys %salad;
foreach (@fruits) {
	my $fruit = $_;
	$fruit =~ s/s$// if $salad{$_} == 1;
	print "$salad{$_} $fruit\n";
}
print <<_HERE_DOC_
Cut and stir the fruit.
_HERE_DOC_
__DATA__
apples 2
pears 1
oranges 3
"""

    def __init__(self):
        KoLanguageBase.__init__(self)
        self._style_info.update(
            _indent_styles = [sci_constants.SCE_PL_OPERATOR],
            _lineup_close_styles = [sci_constants.SCE_PL_OPERATOR,
                                    sci_constants.SCE_PL_VARIABLE_INDEXER,
                                    sci_constants.SCE_PL_REGEX,
                                    sci_constants.SCE_PL_REGSUBST],
            _lineup_styles = [sci_constants.SCE_PL_OPERATOR,
                              sci_constants.SCE_PL_VARIABLE_INDEXER,
                              sci_constants.SCE_PL_REGEX,
                              sci_constants.SCE_PL_REGSUBST],
            _variable_styles = [sci_constants.SCE_PL_SCALAR,
                                sci_constants.SCE_PL_ARRAY,
                                sci_constants.SCE_PL_HASH,
                                sci_constants.SCE_PL_SYMBOLTABLE,
                                sci_constants.SCE_PL_VARIABLE_INDEXER,
                                sci_constants.SCE_PL_STRING_VAR,
                                sci_constants.SCE_PL_STRING_QQ_VAR,
                                sci_constants.SCE_PL_STRING_QX_VAR,
                                sci_constants.SCE_PL_BACKTICKS_VAR,
                                sci_constants.SCE_PL_HERE_QQ_VAR,
                                sci_constants.SCE_PL_HERE_QX_VAR,
                                sci_constants.SCE_PL_REGEX_VAR,
                                sci_constants.SCE_PL_REGSUBST_VAR,
                                sci_constants.SCE_PL_STRING_QR_VAR],
            )
        self.matchingSoftChars["`"] = ("`", self.softchar_accept_matching_backquote)
        self.matchingSoftChars["/"] = ("/", self.softchar_accept_matching_forward_slash)
        self.matchingSoftChars["("] = (")", self.softchar_check_special_then_return_char)
        self.matchingSoftChars["["] = ("]", self.softchar_check_special_then_return_char)
        self._fastCharData = \
            FastCharData(trigger_char=";",
                         style_list=(sci_constants.SCE_PL_OPERATOR,
                                     sci_constants.SCE_UDL_SSL_OPERATOR, ),
                         skippable_chars_by_style={ sci_constants.SCE_PL_OPERATOR : "])",
                                                    sci_constants.SCE_PL_VARIABLE_INDEXER : "])",
                                                    sci_constants.SCE_UDL_SSL_OPERATOR : "])",},
                         for_check=True)
        
    def getVariableStyles(self):
        return self._style_info._variable_styles

    def getLanguageService(self, iid):
        return KoLanguageBase.getLanguageService(self, iid)

    def get_lexer(self):
        if self._lexer is None:
            self._lexer = KoLexerLanguageService()
            self._lexer.setLexer(sci_constants.SCLEX_PERL)
            self._lexer.setKeywords(0, self._keywords)
            self._lexer.supportsFolding = 1
            self._lexer.setProperty('fold.perl.comment.explicit', '0') # explicit folding bugged as of Scintilla 3.7.3
        return self._lexer

    def get_interpreter(self):
        if self._interpreter is None:
            self._interpreter = components.classes["@activestate.com/koAppInfoEx?app=Perl;1"].getService()
        return self._interpreter

    _keywords = [
                 "__DATA__",
                 "__END__",
                 "__FILE__",
                 "__LINE__",
                 "__PACKAGE__",
                 "__SUB__", # Added in perl 5.16
                 "AUTOLOAD",
                 "BEGIN",
                 "CHECK",
                 "CORE",
                 "DESTROY",
                 "END",
                 "INIT",
                 "UNITCHECK",
                 "abs",
                 "accept",
                 "alarm",
                 "and",
                 "atan2",
                 "bind",
                 "binmode",
                 "bless",
                 "break",
                 "caller",
                 "chdir",
                 "chmod",
                 "chomp",
                 "chop",
                 "chown",
                 "chr",
                 "chroot",
                 "close",
                 "closedir",
                 "cmp",
                 "connect",
                 "continue",
                 "cos",
                 "crypt",
                 "dbmclose",
                 "dbmopen",
                 "default",
                 "defined",
                 "delete",
                 "die",
                 "do",
                 "dump",
                 "each",
                 "else",
                 "elsif",
                 "endgrent",
                 "endhostent",
                 "endnetent",
                 "endprotoent",
                 "endpwent",
                 "endservent",
                 "eof",
                 "eq",
                 "eval",
                 "exec",
                 "exists",
                 "exit",
                 "exp",
                 "fc", # Added in perl 5.16
                 "fcntl",
                 "fileno",
                 "flock",
                 "for",
                 "foreach",
                 "fork",
                 "format",
                 "formline",
                 "ge",
                 "getc",
                 "getgrent",
                 "getgrgid",
                 "getgrnam",
                 "gethostbyaddr",
                 "gethostbyname",
                 "gethostent",
                 "getlogin",
                 "getnetbyaddr",
                 "getnetbyname",
                 "getnetent",
                 "getpeername",
                 "getpgrp",
                 "getppid",
                 "getpriority",
                 "getprotobyname",
                 "getprotobynumber",
                 "getprotoent",
                 "getpwent",
                 "getpwnam",
                 "getpwuid",
                 "getservbyname",
                 "getservbyport",
                 "getservent",
                 "getsockname",
                 "getsockopt",
                 "given",
                 "glob",
                 "gmtime",
                 "goto",
                 "grep",
                 "gt",
                 "hex",
                 "if",
                 "import",
                 "include",
                 "index",
                 "int",
                 "ioctl",
                 "join",
                 "keys",
                 "kill",
                 "last",
                 "lc",
                 "lcfirst",
                 "le",
                 "length",
                 "link",
                 "listen",
                 "local",
                 "localtime",
                 "lock",
                 "log",
                 "lstat",
                 "lt",
                 "m",
                 "map",
                 "mkdir",
                 "msgctl",
                 "msgget",
                 "msgrcv",
                 "msgsnd",
                 "my",
                 "ne",
                 "new",
                 "next",
                 "no",
                 "not",
                 "oct",
                 "open",
                 "opendir",
                 "or",
                 "ord",
                 "our",
                 "pack",
                 "package",
                 "pipe",
                 "pop",
                 "pos",
                 "print",
                 "printf",
                 "prototype",
                 "push",
                 "q",
                 "qq",
                 "qr",
                 "qx",
                 "qw",
                 "quotemeta",
                 "rand",
                 "read",
                 "readdir",
                 "readline",
                 "readlink",
                 "readpipe",
                 "recv",
                 "redo",
                 "ref",
                 "rename",
                 "require",
                 "reset",
                 "return",
                 "reverse",
                 "rewinddir",
                 "rindex",
                 "rmdir",
                 "s",
                 "say",
                 "scalar",
                 "seek",
                 "seekdir",
                 "select",
                 "semctl",
                 "semget",
                 "semop",
                 "send",
                 "setgrent",
                 "sethostent",
                 "setnetent",
                 "setpgrp",
                 "setpriority",
                 "setprotoent",
                 "setpwent",
                 "setservent",
                 "setsockopt",
                 "shift",
                 "shmctl",
                 "shmget",
                 "shmread",
                 "shmwrite",
                 "shutdown",
                 "sin",
                 "sleep",
                 "socket",
                 "socketpair",
                 "sort",
                 "splice",
                 "split",
                 "sprintf",
                 "sqrt",
                 "srand",
                 "stat",
                 "state",
                 "study",
                 "sub",
                 "substr",
                 "symlink",
                 "syscall",
                 "sysopen",
                 "sysread",
                 "sysseek",
                 "system",
                 "syswrite",
                 "tell"
                 "telldir",
                 "tie",
                 "tied",
                 "time",
                 "times",
                 "tr",
                 "truncate",
                 "uc",
                 "ucfirst",
                 "umask",
                 "undef",
                 "unless",
                 "unlink",
                 "unpack",
                 "unshift",
                 "untie",
                 "until",
                 "use",
                 "utime",
                 "values",
                 "vec",
                 "wait",
                 "waitpid",
                 "wantarray",
                 "warn",
                 "when",
                 "while",
                 "write",
                 "xor",
                 "y"
                 ]
    
    def softchar_accept_matching_forward_slash(self, scimoz, pos, style_info, candidate):
        if pos == 0:
            return candidate
        currStyle = scimoz.getStyleAt(pos)
        if not currStyle in style_info._regex_styles:
            return None
        prevPos = scimoz.positionBefore(pos)
        if scimoz.getStyleAt(prevPos) != currStyle:
            # We're at the start of a regex.
            return candidate
        # Check for m/ or s/
        if pos >= 2:
            prev2Pos = scimoz.positionBefore(prevPos)
            if scimoz.getStyleAt(prev2Pos) == currStyle:
                return None
        leadChar = scimoz.getWCharAt(prevPos)
        if leadChar == 's':
            return "//"
        elif leadChar == 'm':
            return candidate
        return None
    
    def _is_special_variable(self, scimoz, pos, opStyle):
        if pos == 0:
            return False;
        prevPos = scimoz.positionBefore(pos)
        if scimoz.getStyleAt(prevPos) == opStyle and chr(scimoz.getCharAt(prevPos)) == '$':
            # In Perl $( and $[ have particular meanings
            return True
        return False

    def softchar_check_special_then_return_char(self, scimoz, pos, style_info, candidate):
        if self._is_special_variable(scimoz, pos,
                                     self.isUDL() and scimoz.SCE_UDL_SSL_VARIABLE or scimoz.SCE_PL_SCALAR):
            return None
        return candidate
    
    _is_alpha_re = re.compile(r'\w')
    def _atOpeningStringDelimiter(self, scimoz, pos, style_info):
        res = KoLanguageBase._atOpeningStringDelimiter(self, scimoz, pos, style_info)
        if res:
            return res
        # Look at Perl's special cases
        if pos < 4:
            return False
        # Look for a delim after the q-part
        prevPos = scimoz.positionBefore(pos)
        prevStyle = scimoz.getStyleAt(prevPos)
        if prevStyle not in style_info._string_styles:
            return False
        prevChar = scimoz.getWCharAt(prevPos)
        if self._is_alpha_re.match(prevChar):
            return False
        # Look for a char like rxw before delim
        prevPos = scimoz.positionBefore(prevPos)
        prevStyle = scimoz.getStyleAt(prevPos)
        if prevStyle not in style_info._string_styles:
            return False
        prevChar = scimoz.getWCharAt(prevPos)
        if prevChar not in "qwrx":
            return False
        
        # Look for a q
        prevPos = scimoz.positionBefore(prevPos)
        prevStyle = scimoz.getStyleAt(prevPos)
        if prevStyle in style_info._indent_open_styles:
            return prevChar == 'q'
        elif prevStyle not in style_info._string_styles:
            return False
        prev2Char = scimoz.getWCharAt(prevPos)
        if prev2Char != 'q':
            return False
        
        return self._atOpeningIndenter(scimoz, scimoz.positionBefore(prevPos), style_info)
