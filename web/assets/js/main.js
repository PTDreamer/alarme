  
  function setVisibility (svgName, elementId, visibility) {
    console.log(svgName);
     var svgEmbed = document.embeds[svgName];
     if (typeof svgEmbed.getSVGDocument != 'undefined') {
       var svgDocument = svgEmbed.getSVGDocument();
       var element = svgDocument.getElementById(elementId);
       console.log(svgEmbed);
       console.log(svgDocument)
       if (element != null) {
         element.setAttribute('visibility', visibility);
     }
 }
};
$(document).ready(function(){
    $.getJSON("data/getsvgs.json",
    function (json) {
        var tr;
        var svgfiles = [];
        for (var i = 0; i < json.length; i++) {
            var div = $('<div class="col-md-3"></div>');
            div.append("<h3>" + json[i].friendly + "</h3>");
            div.append("<p><embed name=" + json[i].name + ' type="image/svg+xml" ' + 'src="images/' + json[i].name + '"' + "></p>");
            $('#mapimages').append(div);
        }
    });
    $.getJSON("data/getdevices.json",
    function (json) {
        var tr;
            $.each(json.devices, function(i,v) {
            if(v.isSensor == 1) {
                tr = $("<tr id=sensor" + v.id + "></tr>");
                tr.append("<td>" + v.id + "</td>");
                tr.append("<td>" + v.name + "</td>");
                tr.append('<td class="status">ready</td>');
                tr.append('<td><div class="glyphicon glyphicon-ok"></div></td>');
                $('#sensorlisttable').append(tr);
            }
        });
    });
    $.getJSON("data/getrelations.json",
    function (json) {
        for (var i = 0; i < json.length; i++) {
            var row = $('#sensor' + json[i].sensorid);
            if(row != null) {
                row.attr('svgfile', json[i].svgname);
                row.attr('svgelement', json[i].svgid);
            }
        };
    });
    $.getJSON("data/alarmmodes.json",
    function (json) {
        var option;
        var combo = $("#currentMode")
        for (var i = 0; i < json.length; i++) {
            
            option = $("<option value=" + json[i].id + ">" + json[i].name + "</option>");
            combo.append(option);
        }
    });
    currentMode.change(function(e) {
        handleModeChange();
});
    var intervalID = setInterval(function(){getAlarms();}, 5000);
    var intervalIDD = setInterval(function(){getDisabled();}, 5000);
});
$(function () {
   'use strict';

   $('a[data-toggle="tab"]').on('shown.bs.tab', function (e) {
       var id = $(this).parents('[role="tablist"]').attr('id');
       var key = 'lastTag';
       if (id) {
           key += ':' + id;
       }

       localStorage.setItem(key, $(e.target).attr('href'));
   });

   $('[role="tablist"]').each(function (idx, elem) {
       var id = $(elem).attr('id');
       var key = 'lastTag';
       if (id) {
           key += ':' + id;
       }

       var lastTab = localStorage.getItem(key);
       if (lastTab) {
           $('[href="' + lastTab + '"]').tab('show');
       }
   });
});

function getAlarms() {
    $.ajax({
        type: 'GET',
        url: "data/alarms.json",
        dataType: 'json',
        success: function (data) {
            $.each(data.alarms, function(i, alarm) {
                handleAlarm(alarm);
            });
            handleStatus(data);
            $("#serveralert").hide();
        },
        error: function (data) {
            $("#serveralert").show();
        }
    });
};

function getDisabled() {
    $.ajax({
        type: 'GET',
        url: "data/getdisabled.json",
        dataType: 'json',
        success: function (data) {
            handleDisabled(data);
        },
        error: function (data) {
            $("#serveralert").show();
        }
    });
};

function handleAlarm(alarm) {
    var $row = $("#sensor" + alarm.id);
    var $status = $row.find("td.status");
    var svgfile = $row.attr("svgfile");
    var svgelement = $row.attr("svgelement");
    if(typeof svgfile !== typeof undefined && svgfile !== false) {
        if(alarm.state == "1") {
            $row.addClass("danger");
            $status.html("ALARM");
            console.log(svgfile + svgelement + "VISIBLE");
            setVisibility(svgfile, svgelement, "visible");
        } else {
            $row.removeClass("danger");
            $status.html("ready");
            setVisibility(svgfile, svgelement, "hidden");
        }
    }
};

function handleDisabled(alarm) {
    $.each(alarm, function (idx, elem) {
        var $row = $("#sensor" + elem.id);
        var $icon = $row.find("div.glyphicon");
        if(elem.isDisabled == "1") {
            $icon.addClass("glyphicon-remove");
            $icon.removeClass("glyphicon-ok");
            $row.addClass("info");
        } else {
            $icon.addClass("glyphicon-ok");
            $icon.removeClass("glyphicon-remove");
            $row.removeClass("info");
        }
    });
};

    var statusID = $("#statusID");
    var currentMode = $("#currentMode");
    var forcedModeID = $("#forcedmodeID");

function handleStatus(status) {
    if(status.armed == "1") {
        statusID.removeClass("btn-success");
        statusID.addClass("btn-danger");
        statusID.html("Armed")
    } else {
        statusID.removeClass("btn-danger");
        statusID.addClass("btn-success");
        statusID.html("Disarmed")
    }
    currentMode.val(status.activeMode);
    if(status.forcedMode == "1") {
        forcedModeID.prop('checked', true);
    } else {
        forcedModeID.prop('checked', false);
    }
};

forcedModeID.on('change', function(e) {
    handleForcedChange();
});
// Bind click to OK button within popup
$('#confirm-forcedmode').on('click', '.btn-ok', function(e) {
  var $modalDiv = $(e.delegateTarget);
  var id = $(this).attr('recordId');
  $modalDiv.addClass('loading');
  var data = {};
    if($("#confirm-forcedmode").attr("type") == "modechange") {
        data.forced = 1;
        data.newMode = id;
        $.ajax({
            url: "data/statuschange.json",
            dataType: 'json',
            type: 'post',
            data: JSON.stringify(data),
            contentType: 'application/json',
            processData: false,
            success: function( data, textStatus, jQxhr ){
                if(data.status == "success") {
                    $modalDiv.modal('hide').removeClass('loading');
                    showMessage("success", "Data saved successfully");
                } else {
                    showMessage("error", "An error ocorred while saving data");
                }
            },
            error: function( jqXhr, textStatus, errorThrown ){
                console.log( errorThrown );
                showMessage("error", "Data saving failed. Server apeers to be down");
            }
        })
    } 
    else if($("#confirm-forcedmode").attr("type") == "forcedchange") {
        var v;
        if(forcedModeID.prop('checked') == true) {
            v = "1";
        } else {
            v = "0";
        }
        data.forced = v;
        $.ajax({
            url: "data/statuschange.json",
            dataType: 'json',
            type: 'post',
            data: JSON.stringify(data),
            contentType: 'application/json',
            processData: false,
            success: function( data, textStatus, jQxhr ){
                if(data.status == "success") {
                    $modalDiv.modal('hide').removeClass('loading');
                    showMessage("success", "Data saved successfully");
                } else {
                    showMessage("error", "An error ocorred while saving data");
                }
            },
            error: function( jqXhr, textStatus, errorThrown ){
                console.log( errorThrown );
                showMessage("error", "Data saving failed. Server apeers to be down");
            }
        })
    }
    else if($("#confirm-forcedmode").attr("type") == "armedchange") {
        var v;
        if(statusID.hasClass("btn-success")) {
                v = "1";
            } else {
                v = "0";
            }
            data.armed = v;
            $.ajax({
            url: "data/statuschange.json",
            dataType: 'json',
            type: 'post',
            data: JSON.stringify(data),
            contentType: 'application/json',
            processData: false,
            success: function( data, textStatus, jQxhr ){
                if(data.status == "success") {
                    showMessage("success", "Data saved successfully");
                    $modalDiv.modal('hide').removeClass('loading');
                    if(v == "1") {
                        statusID.removeClass("btn-success");
                        statusID.addClass("btn-danger");
                        statusID.html("Armed")
                    } 
                    else {
                        statusID.removeClass("btn-danger");
                        statusID.addClass("btn-success");
                        statusID.html("Disarmed")
                    }
                } else {
                    showMessage("error", "An error ocorred while saving data");
                }
            },
            error: function( jqXhr, textStatus, errorThrown ){
                console.log( errorThrown );
                showMessage("error", "Data saving failed. Server apeers to be down");
            }
        })
        }
});

$('#confirm-forcedmode').on('click', '.btn-default', function(e) {
    if($("#confirm-forcedmode").attr("type") == "modechange") {
        currentMode.val($(this).attr("recordId"));
    } 
    else if($("#confirm-forcedmode").attr("type") == "forcedchange") {
        if(forcedModeID.prop('checked') == true) {
            forcedModeID.prop('checked', false);
        } else {
            forcedModeID.prop('checked', true);
        }
    }
    else if($("#confirm-forcedmode").attr("type") == "armedchange") {
        //do nothing
    }
});
$('#statusID').on('click', function(e) {
    handleArmedChange();
});
function handleModeChange() { 
    var data = $("#currentMode");
    $("#confirm-forcedmode .modal-text").text("You are about to force the mode to" + $("#currentMode option:selected").text());
    $("#confirm-forcedmode .modal-title").text("Confirm forced mode");
    $("#confirm-forcedmode .btn-ok").attr('recordId', data.val());
    $("#confirm-forcedmode .btn-ok").text("Force");
    $("#confirm-forcedmode .btn-default").attr('recordId', data.attr("oldvalue"));
    $("#confirm-forcedmode").attr('type', "modechange");
    $("#confirm-forcedmode").modal("show");
}

function handleForcedChange() { 
    if(forcedModeID.prop('checked') == true) {
        $("#confirm-forcedmode .modal-text").text("You are about to force the mode to " + $("#currentMode option:selected").text());
        $("#confirm-forcedmode .btn-ok").html("Force");
    } else {
        $("#confirm-forcedmode .modal-text").text("You are about to unforce the current mode");
        $("#confirm-forcedmode .btn-ok").html("UnForce");
    }
    $("#confirm-forcedmode .modal-title").text("Confirm operation");
    $("#confirm-forcedmode").attr('type', "forcedchange");
    $("#confirm-forcedmode").modal("show");
}

function handleArmedChange() { 
    if(statusID.hasClass("btn-success")) {
        $("#confirm-forcedmode .modal-text").text("You are about to Arm the System");
        $("#confirm-forcedmode .btn-ok").html("Arm");
    } else {
        $("#confirm-forcedmode .modal-text").text("You are about to Disarm the System");
        $("#confirm-forcedmode .btn-ok").html("Disarm");
    }
    $("#confirm-forcedmode .modal-title").text("Confirm operation");
    $("#confirm-forcedmode").attr('type', "armedchange");
    $("#confirm-forcedmode").modal("show");
}

$('#currentMode').focus(function(e) {
    $(this).attr("oldvalue", this.value);
});

function showMessage(type, text) {
    var boldText;
    var classTxt;
    if(type == "warning") {
        boldText = "Warning";
        classTxt = alert-warning;
    } else if(type == "error") {
        boldText = "Error";
        classTxt = "alert-danger";
    } else if(type == "info") {
        boldText = "Information";
        classTxt = "alert-info"
    } else if(type == "success") {
        boldText = "Success"
        classTxt = "alert-success";
    }
    var html = $('<div class="alert ' + classTxt + '"><strong>' + boldText + '</strong>' + '  ' + text + '</div>');
    $(".maincontainer").prepend(html);
    $('html, body').animate({scrollTop: '0px'}, 500);
    html.delay(5000).fadeOut(500);
    setTimeout(function() {
        html.remove();
    }, 10000);
}