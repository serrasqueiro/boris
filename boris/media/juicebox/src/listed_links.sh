#!/bin/sh


usage ()
{
 echo "$0 -
OR
   $0 opt_path

Where opt_path is e.g. /tmp/musical/ or just /
If opt_path is 'unnumbered' no leading sequence numbers are displayed.
"
 exit 0
}


listed_links ()
{
 grep ^"L	"
}


absorbe_tabs ()
{
 sed 's/.*	//'
}


absorbe_slash ()
{
 sed 's/.*\///'
}


unnumbered ()
{
 absorbe_slash | sed 's/[^ ]* //'
}



listing ()
{
 local OPT="$1"

 case "$OPT" in
	-)
	listed_links | absorbe_tabs;;

	unnumbered)
	listed_links | unnumbered;;

	*)
	listed_links | absorbe_slash;;
 esac
}



[ "$1" = "" ] && usage


listing "$1"

exit 0

