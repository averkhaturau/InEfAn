import sys

from log_parse import *
from log_plot  import *

def parse_log():
    reload(sys)  
    sys.setdefaultencoding('utf-8')
 #   sys.stdout = open(sys.stdout.fileno(), mode='w', encoding='utf8', buffering=1)

    if len(sys.argv) < 2:
        print("Usage:\n\t>" + sys.argv[0] + " inefan.log")
        exit(-1)

    with open(sys.argv[1]) as logfile:
        for line in logfile:
           handle_log_line(line[:-1])

parse_log()

print_characteristics()
log_plot(key_press_events)