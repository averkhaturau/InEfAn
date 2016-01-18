import sys
import datetime
import re

def main():
    if len(sys.argv) < 2:
        print("Usage:\n\t>" + sys.argv[0] + " inefan.log")
        exit(-1)

    infile = sys.argv[1]

    with open(infile) as logfile:
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
            line_word = filter(None, line.split(" "))
            if len(line_word) < 3:
                return
            event_time = datetime.datetime.strptime( line_word[0] + " " + line_word[1], "%Y-%m-%d %H:%M:%S.%f" )
            print(event_time, line_word[2:])
            if line_word[2] == "mouse":
            	pass
            elif line_word[2] == "keyboard":
            	pass
            else:
            	# system event
            	print (" ".join(line_word[2:]) + " at " + unicode(event_time))

main()