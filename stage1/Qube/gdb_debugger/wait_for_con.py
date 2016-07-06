import subprocess
import time
while subprocess.Popen([r'netstat','-ano'], stdout=subprocess.PIPE, shell=True).communicate()[0].find("TCP    127.0.0.1:8864") == -1:
    pass
time.sleep(1.5)