// Used by Win32Tray.rc
//
#define IDC_MYICON                    2
#define IDD_WIN32TRAY_DIALOG          102
#define IDC_WIN32TRAYAPP              109
#define IDC_LOGFILES_SENT             115
#define IDC_LOGFILES_NOTSENT          116
#define IDR_MAINFRAME                 128
#define IDR_MENU1                     129
#define IDI_ICON1                     131
#define IDI_MAINICON                  131
#define IDI_MAINICONPAUSED            132
#define ID_TRAYMENU_EXIT              32771
#define ID_TRAYMENU_ABOUT             32777
#define ID_TRAYMENU_SENDLOGFILES      32778
#define ID_TRAYMENU_NEW_LOG           32783
#define ID_TRAYMENU_PAUSE             32779
#define ID_TRAYMENU_RESUME            32780
#define ID_TRAYMENU_EDIT_LOGFILE      32784
#define ID_TRAYMENU_OPEN_LOGFILES_DIR 32785
#define ID_TRAYMENU_MYPROFILE         32786
#define ID_TRAYMENU_SHOW_VERSION      32787
#define IDC_LOGGING_PAUSED            32781
#define IDC_LOGGING_RESUMED           32782
#define IDC_SHOW_VERSION              32788

//Do not use values from this range for other purposes
#define ID_SERVERS_LIST_BEGIN         33000
#define ID_SERVERS_LIST_END           33200

#define IDC_STATIC                    -1

#define TRAY_ICON_MESSAGE (WM_APP + 0x0001)
#define RESUME_LOGGING_MESSAGE (WM_APP + 2)
