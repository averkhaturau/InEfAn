import sys
import datetime
import re

def main():
    reload(sys)  
    sys.setdefaultencoding('utf8')

    if len(sys.argv) < 2:
        print("Usage:\n\t>" + sys.argv[0] + " inefan.log")
        exit(-1)

    with open(sys.argv[1]) as logfile:
        for line in logfile:
           handle_log_line(line[:-1])


parsing_header = False

is_using_mouse = False
is_using_keyboard = False

def handle_log_line(line):
    global parsing_header
    if parsing_header:
        if line.startswith("==="):
            parsing_header = False
        else:
            m = re.search("(?<=running on)[ \\t]+(.*)[ \\t]+powered by[ \\t]+(.*)", line)
            if m:
                print("Running on " + m.group(1) + " powered by " + m.group(2))
    else:
        if line.startswith("==="):
            parsing_header = True
        else:
            line_word = filter(None, re.split(" |\t|\n|\r", line))
            if len(line_word) < 3:
                return
            event_time = datetime.datetime.strptime( line_word[0] + " " + line_word[1], "%Y-%m-%d %H:%M:%S.%f" )
            print(event_time, line_word[2:])

            if line_word[2] == "mouse":
            	if line_word[-1] == "up" or line_word[-1] == "finished":
                    print("Mouse using stopped at " + unicode(event_time))
                elif line_word[-1] == "down" or line_word[-1] == "started":
                    print("Mouse using started at " + unicode(event_time))
                else:
                    print("Mouse single event at " + unicode(event_time))

            elif line_word[2] == "keyboard":
            	if line_word[-1] == "up":
                    print("Keyboard using stopped at " + unicode(event_time))
                elif line_word[-1] == "down":
                    print("Keyboard using started at " + unicode(event_time))
                else:
                    print("Keyboard single event at " + unicode(event_time))

            else:
            	# system event
            	#print (u" ".join(line_word[2:]) + " at " + unicode(event_time))
                pass

main()