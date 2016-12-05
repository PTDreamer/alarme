var oldURL = document.referrer;

var pingUrl = oldURL;
var targetUrl = oldURL;
setInterval(ping, 3000);

function ping() {
    $.ajax({
        url: pingUrl,
        success: function(result) {
            window.location.href = targetUrl;
        }
    });
}
$(document).ready(function(){
var data = {};
data.action = "reboot";
$.ajax({
        url: "data/reboot.php",
        dataType: 'json',
        type: 'post',
        data: JSON.stringify(data),
        contentType: 'application/json',
        processData: false,
    })
})