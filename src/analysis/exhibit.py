import sys

fileheader = '''<html>
<head>
    <title>Results: InEfAn Util</title>
    <meta charset="UTF-8">
    <script src="jquery-1.12.0.min.js"></script>

    <link rel="stylesheet" type="text/css" href="stat-slides.css">

<script type="text/javascript">
    $(document).ready(function(){
        for(i=1;i<=@numberofmessages@+1;++i)
            $("#element-" + i).css("display", "none");
        (function myLoop (i) {
                $("#element-" + i).fadeOut("1000");
                ++i;
                $("#element-" + i).delay("500").fadeIn("1000");
                if (i<=@numberofmessages@)
                   setTimeout(function () {myLoop(i);}, 4000)
                else
                   $(".background-image").css("display", "inline")
        })(0);
    });
</script>

</head>
<body>
    <div class="container">
        <div class="middle">
'''

filefooter = '''
        </div>
        <div class="logo">
            <img src="logo_footer.png">
        </div>
    </div>
<div class="background-image" style="display:none;"></div>
</body>
</html>
'''

def present_as_html(msgs,filename):
    if sys.version_info < (3, 0):
        reload(sys)  
        sys.setdefaultencoding('utf8')
        fopen_func = lambda filename: open(filename, "w")
    else:
        fopen_func = lambda filename: open(filename, "w", encoding='utf8')

    with fopen_func(filename) as exhibit_html:
        exhibit_html.write(fileheader.replace("@numberofmessages@",str(len(msgs))))
        i = 0
        for msg in msgs:
            i += 1
            exhibit_html.write('<div id="element-'+ str(i) +'"><h1 class="text">' + msg + '</h1></div>\n')
        i += 1
        exhibit_html.write('<div id="element-'+ str(i) +'"><h3 class="text"><ul>\n')
        for msg in msgs:
            exhibit_html.write('<li>'+msg+'</li>\n')
        exhibit_html.write('</ul></h3></div>\n')

        exhibit_html.write(filefooter)

