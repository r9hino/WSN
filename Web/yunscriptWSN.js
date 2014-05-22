$(document).ready(function(e) {
	// Turn off Ajax caching
    $.ajaxSetup({ cache: false });
    
	// Load table data
	$.getJSON("/arduino/tableData/",function(data){
		
		
		
	});
	
	// Hide the loading/saving floating DIV
    $('#loadingall').hide();

    var isAutoRefreshP4 = false;
    var timerP4;
    
    var isAutoRefreshP5 = false;
    var timerP5;
    
    // page 2 always has the digital pin data directions - load once only   
    // Note, for YUN, keep digital D0 and digital D1 reserved for arduino <-> linux exchange
    // so that pins shown are D2 ... D13
    populate_page2();
    initpage2();        // now initialise their states

// *****************************************************************************    
//                      PAGE 2 FUNCTIONS
// *****************************************************************************    
        
    // set up the radio groups to allow pin data direction selection
    // between output, input and input with internal pull-up enabled
    function populate_page2()
    {
        $('#setdigital_io').empty();    // empty the div

        for (var i=2;i<=13;i++)
        {
            var labStr = "D"+i.toString();
            $('#setdigital_io').append('<div id="radiogroup'+i+'" data-role="fieldcontain"><fieldset data-role="controlgroup" data-type="horizontal" data-mini="false"><legend>'+labStr+'</legend><input type="radio" name="radio-choice-d'+i+'" id="radio-choice-d'+i+'1" value="choice-'+i+'1" checked="checked"/><label for="radio-choice-d'+i+'1">Output</label><input type="radio" name="radio-choice-d'+i+'" id="radio-choice-d'+i+'2" value="choice-'+i+'2"/><label for="radio-choice-d'+i+'2">Input</label><input type="radio" name="radio-choice-d'+i+'" id="radio-choice-d'+i+'3" value="choice-'+i+'3"/><label for="radio-choice-d'+i+'3">Input (PU)</label></fieldset></div>'   );
        }
        $('#setdigital_io').trigger('create');  // trigger a create on parent div to make sure the label and buttons are rendered correctly
        blankpage2();   // reset all choices
    }
    
    // when page 2 selected from the main menu call the function to read the
    // current data directions and update the radio selections 
    $('#callinitp2').click(function() {initpage2();});
    
    // function to make a call to the Yun and use the JSON data sent back to initialise the radio selections
    function initpage2()
    {
        $('#loadingall').html('...Loading');
        $('#loadingall').show();
        //$.getJSON("V_io_test.json",function(data){    // swap this for line below to test locally
        $.getJSON("/arduino/in/",function(data){        // send the in command to the Yun
        var j=2;
        $.each(data.Datadir,        // loop through response and update as required
            function (key,value)
            {
                if (value.datadir==0) {$('#radio-choice-d'+j+'1').prop("checked",true).checkboxradio( "refresh" );}else{$('#radio-choice-d'+j+'1').prop("checked",false).checkboxradio( "refresh" );}
                if (value.datadir==1) {$('#radio-choice-d'+j+'2').prop("checked",true).checkboxradio( "refresh" );}else{$('#radio-choice-d'+j+'2').prop("checked",false).checkboxradio( "refresh" );}
                if (value.datadir==2) {$('#radio-choice-d'+j+'3').prop("checked",true).checkboxradio( "refresh" );}else{$('#radio-choice-d'+j+'3').prop("checked",false).checkboxradio( "refresh" );}
                j++;
            });
            $('#loadingall').hide();
        }); 
    }
    
    // this function unchecks all the radio selections for page 2
    function blankpage2()
    {
        for (var j=2;j<=13;j++)
        {
            $('#radio-choice-d'+j+'1').prop("checked",false).checkboxradio( "refresh" );
            $('#radio-choice-d'+j+'2').prop("checked",false).checkboxradio( "refresh" );
            $('#radio-choice-d'+j+'3').prop("checked",false).checkboxradio( "refresh" );
        }
    }
        
    // Send new data direction to Yun
    // string sent to arduino is of the form: /arduino/io/012012012012/
    //  0: pin is output
    //  1: pin is input
    //  2: pin is input with pull-up
    $('#save_io').click(function() {
        var urlStr="/arduino"+doSaveStateDir();
        
        $('#loadingall').html('...Saving');
        $('#loadingall').show();
    
        //$.getJSON("stat.json",function(data){ // swap this for line below to test locally
        $.getJSON(urlStr,function(data){
            //alert(data.ret);
            $('#loadingall').hide();
        });
    });
    
    // construct the save-state string to send
    function doSaveStateDir(){  
        var RVal="/io/";
        for (var j=2;j<=13;j++)
        {
            RVal+=getRadioStateDDir('#radio-choice-d'+j);
        }
        RVal+="/";
        
        return RVal;
    }           
    
    // this returns a value of 0, 1 or 2 depending on the selection in the given radio group 
    function getRadioStateDDir(RGSelection)
    {
        var k = 0;
        k = 1*Number($(RGSelection+'1').prop("checked"));
        k += 2*Number($(RGSelection+'2').prop("checked"));
        k += 3*Number($(RGSelection+'3').prop("checked"));
        return (k-1).toString();    
    }       

// *****************************************************************************
//                      PAGE 3 FUNCTIONS
// *****************************************************************************

    // when page 3 selected from the main menu call the function to read the
    // current digital output values and update the radio selections
    $('#callinitp3').click(function() {initpage3();});
    
    // function to make a call to the Yun and use the JSON data sent back to initialise the
    // radio selections for the current digital output values
    function initpage3()
    {
        $('#loadingall').html('...Loading');
        $('#loadingall').show();	// Display ...Loading on webpage

        //$.getJSON("V_io_test.json",function(data){    // swap this for line below to test locally
        $.getJSON("/arduino/in/",function(data){
        $('#setdigital_vals').empty();  // empty the div
        var j=2;
        $.each(data.Digital,
            function (key,value)    // 0/1 digital pin is output with value 0/1     10/11 digital pin is input with value 0/1
            {
                var labStr = "D"+j.toString();
                
				// Only to ouputs we assign radio (button). Input which are 10 or 11 do not enter here...
                if (value.dataval == 0 || value.dataval == 1) 
                {
                    $('#setdigital_vals').append(
						'<div id="radiogroup'+j+'" data-role="fieldcontain">\
							<fieldset data-role="controlgroup" data-type="horizontal" data-mini="true">\
								<legend>'+labStr+'</legend>\
								<input type="radio" name="radio-val-d'+j+'" id="radio-val-d'+j+'1" value="val-'+j+'1" checked="checked"/>\
									<label for="radio-val-d'+j+'1">On</label>\
								<input type="radio" name="radio-val-d'+j+'" id="radio-val-d'+j+'2" value="val-'+j+'2"/>\
									<label for="radio-val-d'+j+'2">Off</label>\
							</fieldset>\
						</div>'
					);

                    $('#setdigital_vals').trigger('create');
                    
                    if (value.dataval == 1){$('#radio-val-d'+j+'1').prop("checked",true).checkboxradio( "refresh" );}
					else{$('#radio-val-d'+j+'1').prop("checked",false).checkboxradio( "refresh" );}
                    
					if (value.dataval == 0){$('#radio-val-d'+j+'2').prop("checked",true).checkboxradio( "refresh" );}
					else{$('#radio-val-d'+j+'2').prop("checked",false).checkboxradio( "refresh" );}        
                }
                j++;
            });
            $('#loadingall').hide();
        }); // getJSON
        $('#setdigital_vals').trigger('create');    // trigger a create on parent div to make sure the label and buttons are rendered correctly
        // going through radio objects here won't work as the getJASON is async and items won't be defined.
    }

    // Send new data values to Yun
    // string sent to arduino is: /arduino/do/010101010101/
    //
    //  0: set pin LOW if output
    //  1: set pin HIGH if output
    
    $('#update_io').click(function() {
        var urlStr = "/arduino"+doSaveStateOut();
        
        $('#loadingall').html('...Saving');
        $('#loadingall').show();
        
        //$.getJSON("stat.json",function(data){ // swap this for line below to test locally
        $.getJSON(urlStr,function(data){
            //alert(data.ret);
            $('#loadingall').hide();
        });
    });
    
    // construct the save-state string to send
    function doSaveStateOut()
	{
        var RVal = "/do/";
    
        for (var j=2; j<=13; j++)
        {
            if ($('#radio-val-d'+j+'1').length > 0)
            {
                RVal += getRadioStateDVal('#radio-val-d'+j);
            }
            else
            {
                RVal += "-";
            }
        }   
        RVal += "/";
        return RVal;
    }

    // this returns a value of 0 or 1 depending on the selection in the given radio group 
    function getRadioStateDVal(RGSelection)
    {
        var k = 0;
        k = 1*Number( $(RGSelection+'2')[0].checked );	// Faster than $(RGSelection+'2').prop("checked")
        k += 2*Number( $(RGSelection+'1')[0].checked ); // Faster than $(RGSelection+'1').prop("checked")
        return (k-1).toString();    
    }

});
