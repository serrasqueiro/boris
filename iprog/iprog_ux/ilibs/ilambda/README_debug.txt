libilambda debug
+++

Intro
+++
debug.cpp is the debug program for the library ilambda, produced using
	'make debug'.

Run 'debug'.

It runs dbg_test_hash():
	actually 'no test'.

Then runs
	dbg_test_string_hash( "." ), must be: 77496

Then runs:
	dbg_test_string_hash( first_arg )
which shows the hash of first_arg, modulus PRIME_MILLION_NP0
	(this is defined at imath.h as 999983).

If no argument is given, it tests 'debug.cpp' file itself.
Otherwise, see the whole text below.

It displays the file_sumA() of the file given by first_arg.

If first_arg is 'json', it parses the file provided in second_arg:
	dbg_parse_json(...)
and exits.

If first_arg is any other string, it will take the second_arg as the fields
descriptions.

Then "DBA aDb.InitDB()" is called, which actually does not do anything for the
user.

So, calling 'debug iJson.cpp .', for instance, will show CRC32 (sumA) of this 
file ('iJson.cpp'), and the default fields provided by 'STR_SAMPLE_DBA_FIELDS'.
