$(document).ready(function() {

//-----------------------------------------------------------------------------------
//	1.	Clock
//-----------------------------------------------------------------------------------

var monthNames = [ "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" ]; 
var dayNames= ["Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"]

var newDate = new Date();
newDate.setDate(newDate.getDate());
$('#DateAbbr').html(dayNames[newDate.getDay()].substr(0,3) + " ");

setInterval( function() {
	var minutes = new Date().getMinutes();
	$(".min, .mins").html(( minutes < 10 ? "0" : "" ) + minutes);
    },1000);
	
setInterval( function() {
	var hours = new Date().getHours();
	$(".hours, .hour").html(( hours < 10 ? "0" : "" ) + hours);
    }, 1000);
	
$('#page, #head').queue(function() { $(this).addClass('vis'); $(this).dequeue(); });
$('.window').queue(function() { $(this).addClass('windows-vis'); $(this).dequeue(); });

//-----------------------------------------------------------------------------------
//	3.	Draggable Windows
//-----------------------------------------------------------------------------------

$('.content').remove();

var a = 3;
$('.content,.specific,.project,.share').draggable({ handle: '.title-inside', start: function(event, ui) { $(this).css("z-index", a++); }});
$(".window").draggable({ handle: '.titleInside, .title-mac, .tab', refreshPositions: true, containment: 'window', start: function(event, ui) { $(this).css("z-index", a++); } });


//-----------------------------------------------------------------------------------
//	4.	Dock
//-----------------------------------------------------------------------------------

$('.dock ul li').hover(
	function(){
		$(this).addClass('ok').prev().addClass('prev').prev().addClass('prev-ancor');
		$(this).addClass('ok').next().addClass('next').next().addClass('next-ancor');
	},
	function(){
		$('.dock ul li').removeClass('ok prev next next-ancor prev-ancor');
	}
);

//-----------------------------------------------------------------------------------
//	5.	Hide and Close
//-----------------------------------------------------------------------------------
var left = 50 + '%';
var top = 15 + '%';
var item = $('<div class="fresh"></div>').hide();
var itemR = $('<div class="fresh"></div>').hide();

$("a[data-rel=close]").click(function(e) {
    e.preventDefault();
    $(this.hash).fadeOut(200, function() {
		$(this).css({ top: top, left: left });
	});
});

$("a[data-rel=show]").click(function(e) {
    e.preventDefault();
    $(this.hash).toggle();
});

$(".dock li a[data-rel=showOp]").click(function(e) {
    e.preventDefault();
	$(this).addClass('bounce').delay(1600).queue(function() { $(this).removeClass('bounce'); $(this).append(item); item.fadeIn(500); $(this).dequeue(); });
    $("#bag").delay(1630).queue(function() { $(this).show(); $(this).dequeue(); });
});

$("#warning a[data-rel=close]").click(function(e) {
    e.preventDefault();
	item.fadeOut(500);
    $(this.hash).hide();
});

$(".dock li a[data-rel=showOpTrash]").click(function(e) {
    e.preventDefault();
	$(this).addClass('bounce').delay(1600).queue(function() { $(this).removeClass('bounce'); $(this).append(itemR); itemR.fadeIn(500); $(this).dequeue(); });
    $("#trash").delay(1630).queue(function() { $(this).show(); $(this).dequeue(); });
});

$("#trash a[data-rel=close]").click(function(e) {
    e.preventDefault();
	itemR.fadeOut(500);
    $(this.hash).hide();
});


}); 