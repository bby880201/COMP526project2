
# coding: utf-8

# In[1]:

import os,sys
import subprocess, shlex 
stats = open('pro2.stat','w')
print >> stats,'LOADFP_CYCLES\tinstructions retired\tExecution completed time\tRetirement completed  at time\tthe number of RFFull stall cycles'
a=1
while a<=128:
    with open('./global.h') as header:
        with open('tmp','w') as ot:
            for line in header:
                if line.startswith('#LOADFP_CYCLES'):
                        words = line.split()
                        words[2] = str(a)
                        print>>ot, ' '.join(words)
                else: ot.write(line)

    subprocess.call('mv tmp global.h', shell=True)

    output = subprocess.Popen(shlex.split('gcc *.c ./yacsim.o -lm -o runme'),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    output.wait()
    output = subprocess.Popen(shlex.split('./runme'),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    output.wait()
    
    NIR = 0
    ECT = 0
    RCT = 0
    RSFull = 0
    
    for line in output.stdout:
        if line.startswith('Number Instructions Retired:'):
            NIR = line.split()[-1]
        elif line.startswith('Retirement completed  at time'):
            RCT = line.split()[-1]
        elif line.startswith('Execution completed at time'):
            ECT = line.split()[-1]
        elif line.startswith('Number Stall Cycles: HALT'):
            col = line.split()
            RSFull = col[8]

    print >>stats, '\t'.join([a,NIR,ECT,RCT,RSFull])
    
    a*=2
stats.close()


# In[ ]:



