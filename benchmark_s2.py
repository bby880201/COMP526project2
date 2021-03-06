import os,sys
import subprocess, shlex
stats = open(sys.argv[1],'w')
print >> stats,'NUM_RESERVATION_STATIONS\tNUM_COPIES\tinstructions retired\tExecution completed time\tRetirement completed  at time\tthe number of RFFull stall cycles'
a=1
NUM_COPIES = [4,64]
for b in NUM_COPIES:
    while a<=5:
        with open('./global.h') as header:
            with open('tmp','w') as ot:
                for line in header:
                    if line.startswith('#define NUM_COPIES'):
                        words = line.split()
                        words[2] = str(a)
                        print>>ot, ' '.join(words)
                    elif line.startswith('#define NUM_RESERVATION_STATIONS'):
                        words = line.split()
                        words[2] = str(b)
                        print>>ot, ' '.join(words)
                    else: ot.write(line)

        subprocess.call('mv tmp global.h', shell=True)
        subprocess.call('gcc *.c ./yacsim.o -lm -o runme', shell=True)
        subprocess.call('./runme > tempOut', shell=True)
    
    #     output = subprocess.Popen('gcc *.c ./yacsim.o -lm -o runme',shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    #     output.wait()
    #     for i in output.stderr:
    #         print i
    #     output = subprocess.Popen('./runme',shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    #     output.wait()
    
        NIR = 0
        ECT = 0
        RCT = 0
        RSFull = 0
    
        with open('tempOut') as output:
            for line in output:
                if line.startswith('Number Instructions Retired:'):
                    NIR = line.split()[-1]
                elif line.startswith('Retirement completed  at time'):
                    RCT = line.split()[-1]
                elif line.startswith('Execution completed at time'):
                        ECT = line.split()[-1]
                elif line.startswith('Number Stall Cycles: HALT'):
                    col = line.split()
                    RSFull = col[8]
        print >>stats, '\t'.join([str(b),str(a),NIR,ECT,RCT,RSFull])
        a+=1
    a=1
stats.close()
