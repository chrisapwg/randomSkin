import sys
import telnetlib

port = 20230

if len(sys.argv) > 1:
    port = sys.argv[1]

try:
    tn = telnetlib.Telnet("localHost", port)
    tn.write('catchQuite(`loadPlugin "learningA"`);'.encode())
    tn.close()
except:
    pass