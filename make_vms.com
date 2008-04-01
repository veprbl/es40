$ SET VERIFY
$!
$! ES40 Emulator
$! Copyright (C) 2007-2008 by the ES40 Emulator Project
$!
$ SAY = "WRITE SYS$OUTPUT"
$!
$ SAY "Configuring the ES40 Emulator..."
$!
$ BASH -c configure_1.sh
$!
$ SAY "  Configuration complete."
$!
$ SAY "Building the ES40 Emulator..."
$!
$ SET DEF [.SRC]
$ @MAKE_VMS.COM
$ SET DEF [-]
$!
$ SAY "  Build complete."
$!
$ EXIT
