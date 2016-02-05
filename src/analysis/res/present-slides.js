
    $(document).ready(function(){
        for(i=1;i<=numberofmessages+1;++i)
            $("#element-" + i).css("display", "none");
        (function myLoop (i) {
                $("#element-" + i).fadeOut("1000");
                ++i;
                $("#element-" + i).delay("500").fadeIn("1000");
                if (i<=numberofmessages)
                   setTimeout(function () {myLoop(i);}, 6000)
                else
                   $(".background-image").css("display", "inline")
        })(0);
    });