#!/bin/sh
bindir=$(pwd)
cd /Users/yidingjiang/Documents/cs184/final/cs184-final/tutorial09_vbo_indexing/
export 

if test "x$1" = "x--debugger"; then
	shift
	if test "x" = "xYES"; then
		echo "r  " > $bindir/gdbscript
		echo "bt" >> $bindir/gdbscript
		GDB_COMMAND-NOTFOUND -batch -command=$bindir/gdbscript  /Users/yidingjiang/Documents/cs184/final/cs184-final/build/tutorial09_several_objects 
	else
		"/Users/yidingjiang/Documents/cs184/final/cs184-final/build/tutorial09_several_objects"  
	fi
else
	"/Users/yidingjiang/Documents/cs184/final/cs184-final/build/tutorial09_several_objects"  
fi
