var shutUP = false;
$(document).ready(function(){
    getDevices();
    getSVGs();
    setInterval(function(){periodicEvents();}, 10000);
});

$('#newline').click(function(e) {
    var newRow = $("#rowtemplate").clone(true);
    newRow.appendTo($("#tablebody"));
    newRow.removeAttr("id");
    newRow.show();
    soa = newRow.find('.sensorOrActuator');
    refreshDeviceTypes(soa);
});
$('.delete').click(function(e) {
    $(this).closest("tr").remove();
});
$('.fileDelete').click(function(e) {
    $(this).closest("tr").addClass('danger');
});
$('.sensorOrActuator').change(function(e) {
	refreshDeviceTypes($(this));
});

function refreshDeviceTypes(device) {
	if(device.val() == 'sensor') {
		device.closest('tr').find('.deviceType')
		    .find('option')
		    .remove()
		    .end()
		    .append('<option value="remote">Remote</option>')
		   	.append('<option value="radioGeneric">Generic Radio Sensor</option>')
		   	.append('<option value="radioPIR">PIR Radio Sensor</option>')
		   	.append('<option value="wiredPIR">Wired PIR Sensor</option>')
		   	.append('<option value="wiredGeneric">Generic Wired Sensor</option>')
		   	.append('<option value="radioReed">Reed Radio Sensor</option>')
		    ;
	} else {
		device.closest('tr').find('.deviceType')
		    .find('option')
		    .remove()
		    .end()
		    .append('<option value="radioSiren">Radio Siren</option>')
		   	.append('<option value="radioPreAlarm">Radio Pre Alarm</option>')
		   	.append('<option value="wiredSiren">Wired Siren</option>')
		   	.append('<option value="wiredPreAlarm">Wired Pre Alarm</option>')
		    ;
	}
}

function getDevices() {
    $.ajax({
        url: "data/getdevices.json",
        dataType: 'json',
        type: 'get',
        contentType: 'application/json',
        processData: false,
        success: function( data, textStatus, jQxhr ){
            $("#preAlarmTime").val(data.preAlarmTime);
            $("#alarmTime").val(data.alarmTime);
            $('#tablebody').find("tr[id!='rowtemplate']").remove();
            $.each(data.devices, function(device) {
                var newRow = $("#rowtemplate").clone(true);
                newRow.removeAttr("id");
                newRow.appendTo($("#tablebody"));
                newRow.find(".ioId").text(this.id);
                newRow.find(".ioName").val(this.name);
                if(this.isSensor == 1) {
                	newRow.find(".sensorOrActuator").val("sensor");
                } else {
                	newRow.find(".sensorOrActuator").val("actuator");
                }
                refreshDeviceTypes(newRow.find(".sensorOrActuator"));
                newRow.find(".deviceType").val(this.type);
                newRow.find(".deviceAddress").val(this.address);
                newRow.find(".mqttAddress").val(this.mqtt);
                disabled = (this.globallyDisabled == 1);
               	newRow.find(".isDisabled").prop('checked', disabled);
                newRow.show();
            });
        },
        error: function( jqXhr, textStatus, errorThrown ){
            console.log( errorThrown );
            showMessage("error", "Data fetching failed. Server apeers to be down");
        }
    })
}


function getSVGs() {
    $.ajax({
        url: "data/getsvgs.json",
        dataType: 'json',
        type: 'get',
        contentType: 'application/json',
        processData: false,
        success: function(data, textStatus, jQxhr ){
        	div = $("#dummySvgContainer");
        	$('#filetable').find("tr[id!='fileRowTemplate']").remove();
            $("#relationsTemplate").find(".relationsSvg").find('option').remove();
            $.each(data, function(index) {
                var newRow = $("#fileRowTemplate").clone(true);
                newRow.removeAttr("id");
                newRow.appendTo($("#filetable"));
                newRow.find(".filename").text(this.name);
                newRow.find('.svgLabel').val(this.friendly);
                newRow.show();
                $("#relationsTemplate").find(".relationsSvg").append('<option>' +this.name+'</option>');
                var svgEmbed = document.embeds[this.name];
     			if (typeof svgEmbed == 'undefined') {
                	div.append('<p class="hiddensvgcontainer"><embed name=' + this.name + ' class="hiddensvg"' + ' type="image/svg+xml" ' + 'src="images/' + this.name + '"' + "></p>");
                }
            });
            getRelations();
        },
        error: function( jqXhr, textStatus, errorThrown ){
            console.log( errorThrown );
            showMessage("error", "Data fetching failed. Server apeers to be down");
        }
    })
}

function getRelations() {
	var allLoaded = true;
	$('.hiddensvg').each(function() {
		var svgEmbed = document.embeds[$(this).attr('name')];
		var doc = svgEmbed.getSVGDocument();
		if(doc == null)
			allLoaded = false;
	});
	if(allLoaded == false) {
		setTimeout(function() {
        getRelations();
    }, 1000);
		return;
	}
    $.ajax({
        url: "data/getrelations.json",
        dataType: 'json',
        type: 'get',
        contentType: 'application/json',
        processData: false,
        success: function(svg, textStatus, jQxhr ){
        	$('#relationstable').find("tr[id!='relationsTemplate']").remove();
        	$('#relationsTemplate').find('.relationsSensorID').find('option').remove();
        	$('.sensorOrActuator').each(function() {
				if($(this).val() == 'sensor') {
					if($(this).closest('tr').attr('id') != 'rowtemplate')
					$('.relationsSensorID').append('<option>' + $(this).closest('tr').find('.ioId').text()+'</option>');
				}
			});
            $.each(svg, function(device) {
            	var newRow = $("#relationsTemplate").clone(true);
                newRow.removeAttr("id");
                newRow.appendTo($("#relationstable"));
                newRow.find(".relationsSvg").val(this.svgname);
            	updateSvgIDs(newRow.find('.relationsSvg'));
				newRow.find(".relationsSvgID").val(this.svgid);
				newRow.find('.relationsSensorID').val(this.sensorid)
                newRow.show();
			});
        },
        error: function( jqXhr, textStatus, errorThrown ){
            console.log( errorThrown );
            showMessage("error", "Data fetching failed. Server apeers to be down");
        }
    })
}
$('.relationsSvg').change(function(e) {
	updateSvgIDs($(this));
});
$('#newRelationsline').click(function(e) {
	var newRow = $("#relationsTemplate").clone(true);
                newRow.removeAttr("id");
                newRow.appendTo($("#relationstable"));
                updateSvgIDs(newRow.find('.relationsSvg'));
                newRow.show();
});
function updateSvgIDs(svgselector) {
	var select = svgselector.closest('tr').find('.relationsSvgID');
	select.find('option').remove().end();
	var svgEmbed = document.embeds[svgselector.val()];
	if (typeof svgEmbed != 'undefined') {
		var svgDocument = svgEmbed.getSVGDocument();
		if (typeof svgDocument != 'undefined') {
			$(svgDocument).find("[id]").each(function(index) {
				select.append('<option>' + $(this).attr("id")+'</option>');
			});
		}
	}
}
$('#svgSave').click(function(e) {
    var hasError = false;
    var svgData = {
        "toDelete" : [],
        "labels" : [],
    };

    $("#filetable").find("tr").each(function (index) {
            if($(this).attr("id") != "fileRowTemplate") {
                if($(this).hasClass('danger')) {
                    svgData.toDelete.push($(this).find('.filename').text());
                }
                else {
                    var label = {
                        "name" : "",
                        "label" : "",
                    };
                    label.name = $(this).find('.filename').text();
                    label.label = $(this).find('.svgLabel').val();
                    svgData.labels.push(label);
                }
            }
    });
    genericSave(svgData, "data/svg.json");
});

$('#relationsSave').click(function(e) {
    var hasError = false;
    var relations = [];
    $("#relationstable").find("tr").each(function (index) {
            if($(this).attr("id") != "relationsTemplate") {
                var relation = {
                "svgname" : '',
                "svgid" : '',
                "sensorid" : 0
                };
                relation.svgname = $(this).find('.relationsSvg').val();
                relation.svgid = $(this).find('.relationsSvgID').val();
                relation.sensorid = Number($(this).find('.relationsSensorID').val());
                relations.push(relation);
            }
        });
        genericSave(relations, "data/relations.json");
});
function genericSave(data, murl) {
    $.ajax({
        url: murl,
        dataType: 'json',
        type: 'post',
        data: JSON.stringify(data),
        contentType: 'application/json',
        processData: false,
        success: function( data, textStatus, jQxhr ){
            if(data.status == "success") {
                showMessage("success", "Data saved successfully");
                if(this.url == "data/svg.json") {
                    getSVGs();
                } else if(this.url == "data/devices.json") {
                    getDevices();
                    getSVGs();
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
$('#save').click(function(e) {
    var hasError = false;
    var dev = {
        "devices" : [],
        "alarmTime" : 0,
        "preAlarmTime" : 0
        };
    dev.alarmTime = Number($("#alarmTime").val());
    dev.preAlarmTime = Number($("#preAlarmTime").val());
   	dev.devices = [];
        $("#tablebody").find("tr").each(function (index) {
            if($(this).attr("id") != "rowtemplate") {
                var record = {
                            "id" : 0,
                            "name" : "",
                            "isSensor" : 0,
                            "type" : "",
                        	"address" : "",
                        	"mqtt" : "",
                        	"isDisabled" : 0};
                record.id = Number($(this).find(".ioId").text());
                record.name = $(this).find(".ioName").val();
                if($(this).find(".sensorOrActuator").val() == "sensor") {
                	record.isSensor = 1;
                } else {
                	record.isSensor = 0;
                }
               	record.type = $(this).find(".deviceType").val();
                record.address = Number($(this).find(".deviceAddress").val());
                record.mqtt = $(this).find(".mqqtAddress").val();
                if($(this).find(".isDisabled").prop('checked')) {
                    record.isDisabled = 1;
                } else {
                    record.isDisabled = 0;
                }
                dev.devices.push(record);
            }
        });
        genericSave(dev, "data/devices.json");
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
function getTime() {
    $.getJSON("data/gettime.json",
        function (json) {
            $("#timedisplay").text(json.hour + ":" + json.minute + ":" + json.seconds);
        });
}
function periodicEvents() {
    if(shutUP == true)
        return;
    getTime();
    getTestStatus();
}
function getTestStatus() {
    $.getJSON("data/getteststatus.json",
        function (json) {
        	if(json.testRadio == 1)
        		$("#testRadio").bootstrapToggle('on');
        	else
        		$("#testRadio").bootstrapToggle('off');
        	if(json.testWired == 1)
        		$("#testWired").bootstrapToggle('on');
        	else
        		$("#testWired").bootstrapToggle('off');

        	$("#receivedData").text(json.receivedData);
        });
}
$('#testRadio').click(function() {
	console.log('Toggle: ' + $(this).prop('checked'));
});
$('#testRadioW').click(function(e){
    var data = {};
    data.testRadio = 0;
    e.stopPropagation();
    if($("#testRadio").prop('checked') == true) {
    	data.testRadio = 0;
    }
    else
    	data.testRadio = 1;
        $.ajax({
        dataType: "json",
        url: "data/getteststatus.json",
        data: data
    });
});
$('#testWiredW').click(function(e){
    var data = {};
    data.testWired = 0;
    e.stopPropagation();
    if($("#testWired").prop('checked') == true) {
    	data.testWired = 0;
    }
    else
    	data.testWired = 1;
        $.ajax({
        dataType: "json",
        url: "data/getteststatus.json",
        data: data
    });
});
$('#send').click(function(e){
    var data = {};
    data.address = Number($("#sendAddress").val());
    data.type = $("#sendType").val();
    data.value = Number($("#sendValue").val());
        $.ajax({
        dataType: "json",
        url: "data/getteststatus.json",
        data: data
    });
});

$('#fileSelectButton').change(function(){
    var file = this.files[0];
    var name = file.name;
    var size = file.size;
    var type = file.type;
    //Your validation
});

$('#fileUploadButton').click(function(){
    shutUP = true;
    var formData = new FormData();
    formData.append("upfile", $("#fileSelectButton").prop('files')[0]);
    $.ajax({
        url: 'data/upload.php',  //Server script to process data
        type: 'POST',
        xhr: function() {  // Custom XMLHttpRequest
            var myXhr = $.ajaxSettings.xhr();
            if(myXhr.upload){ // Check if upload property exists
                myXhr.upload.addEventListener('progress',progressHandlingFunction, false); // For handling the progress of the upload
            }
            return myXhr;
        },
        //Ajax events
       // beforeSend: beforeSendHandler,
        success:  function(data, textStatus, jqXHR ) {
            console.log("success");
            console.log(data);
            console.log(textStatus);
            showMessage("success", "File upload completed.");
            var newRow = $('#fileRowTemplate').clone();
            newRow.removeAttr("id");
            newRow.appendTo($("#filetable"));
            newRow.find(".filename").text($('#fileSelectButton').prop('files')[0].name);
            newRow.show();
            getSVGs();
            shutUP = false;
        },
        error: function( jqXhr, textStatus, errorThrown ){
            console.log( errorThrown );
            showMessage("error", "File upload failed. Server apeers to be down");
            shutUP = false;
        },
        // Form data
        data: formData,
        //Options to tell jQuery not to process data or worry about content-type.
        cache: false,
        contentType: false,
        processData: false
    });
});
function progressHandlingFunction(e){
    if(e.lengthComputable){
        $('progress').attr({value:e.loaded,max:e.total});
    }
}