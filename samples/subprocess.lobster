let ret, out = launch_subprocess([ "ipconfig" ])
if ret < 0:
    print "couldn\'t launch subprocess: {ret}"
else:
    print "process returned: {ret}"
    print "---"
    print out
