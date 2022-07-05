#!/usr/bin/python

import sys
import subprocess

# info line *0x8088
# gdb -nx -n batch -x ./backtrace.cmb
with open("./backtrace.cmd",'w') as cf:
    for arg in sys.argv[1:]:
        cf.write("info line *%s\n" % (arg))
subprocess.call(["gdb", "./kernel7.elf", "-nx", "-n", "-batch", "-x", "./backtrace.cmd"])


