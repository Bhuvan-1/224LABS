$(function() {
	if (typeof $.fn.lazyload === 'undefined' ) {
		if ('loading' in HTMLImageElement.prototype) {
		    const images = document.querySelectorAll('img[loading="lazy"]');
		    images.forEach(img => {
		      img.src = img.dataset.original;
		    });
		}
		else {
			$.getScript('/www/js/jquery/jquery.lazyload.min.js',function(){
				 $("img.lazy").lazyload({ 
				    	effect : "fadeIn", 
				    	threshold: 200,
				    	failure_limit:25
				 });
			});
		}
	}
});

/* global $(document).ready() for every IDGE site */

$(document).ready(function(){
		
	//close hamburger nav and most read when clicked outside of - Mule code had this only in the article js file
	 $('body').click(function(e){
		 $('#banner').removeClass('nav-open');
         $('#reading-selector').removeClass('open');
         $('.reading').removeClass('open');
         $('#insider-selector').removeClass('open');
         $('.insider-list').removeClass('open');
         $(".user-info .user-actions").removeClass("open");
         $('nav#sticky-nav li').removeClass('active arrow');
         $('nav#sticky-nav li ul.subnav').css('display', 'none');
     });
	 //for tablets with touch events
 	$('body').on('touchstart', function(e) {
	    $('#banner').removeClass('nav-open');
	    $('nav#sticky-nav li ul.subnav').css('display', 'none');
	})
	$("#banner").on('touchstart',function(e) {
	    e.stopPropagation();
	});
	 
	
	 $('nav#sticky-nav > ul > li > a.notMobile').click(function() {
	  		var checkElement = $(this).next();

	  		$('nav#sticky-nav li').removeClass('active');
	  		$(this).closest('li').addClass('active'); 
	  		$('nav#sticky-nav li').removeClass('arrow');
	  		$(this).closest('li').addClass('arrow');

			if((checkElement.is('ul')) && (checkElement.is(':visible'))) {
	    		//$(this).closest('li').removeClass('active');
				$(this).closest('li').removeClass('arrow');
	    		checkElement.slideUp('fast');
	  		}
	  		if((checkElement.is('ul')) && (!checkElement.is(':visible'))) {
	    		$('nav#sticky-nav ul ul.subnav:visible').slideUp('fast');
	    		checkElement.slideDown('fast');
	  		}

	  		if($(this).closest('li').find('ul').children().length == 0) {
	    		return true;
	  		} else {
	    		return false; 
	  		}

		});
	 

	 
	 //click event for BrandPost "Learn More" buttons
	 $(".learn-more").click(function(event) {
		 var brandpost_txt = "<div class=\"about-brandposts\"><p>BrandPosts are written and edited by members of our sponsor community. BrandPosts create an opportunity for an individual sponsor to provide insight and commentary from their point-of-view directly to our audience. The editorial team does not participate in the writing or editing of BrandPosts.</p><hr/><a class=\"close-bar\" href=\"#\">Close</i></a></div>";
		 if ($(".blog-byline.sponsored-post").length > 0) { //for Nativo sponsoredPosts
			 brandpost_txt = "<div class=\"about-brandposts\"><p>SponsoredPosts are written and edited by members of our sponsor community. SponsoredPosts create an opportunity for an individual sponsor to provide insight and commentary from their point-of-view directly to our audience. The editorial team does not participate in the writing or editing of SponsoredPosts.</p><hr/><a class=\"close-bar\" href=\"#\">Close</i></a></div>";
		 }
		 $("body").append(brandpost_txt);
		 $(".about-brandposts").css("top",event.pageY);
		 if ($("body").width() >= 955) { // need different behavior at desktop breakpoint and above
			 //find difference between X position clicked and body width and set as right attr 
			 //this is so tooltip doesn't overflow window
			 var xpos = $("body").width() - event.pageX;
			 $(".about-brandposts").css({"right":xpos,"margin-right":"0"});
		 }
		 $(".about-brandposts .close-bar").click(function() {
			 $(".about-brandposts").remove();
			 return false;
		 });
		 return false;
	 });
	 
	//click event for Sponsorship "Learn More" buttons
	 $(".learn-more-sponsorship").click(function(event) {
		 var sponsorship_txt = "<div class=\"about-sponsorships\"><p>This content is written and edited by members of our sponsor community. Such sponsor content creates an opportunity for an individual sponsor to provide insight and commentary from their point-of-view directly to our audience. The editorial team does not participate in the writing or editing of sponsor content.</p><hr/><a class=\"close-bar\" href=\"#\">Close</i></a></div>";
		 if ($(".blog-byline.sponsored-post").length > 0) { //for Nativo sponsoredPosts
			 sponsorship_txt = "<div class=\"about-sponsorships\"><p>SponsoredPosts are written and edited by members of our sponsor community. SponsoredPosts create an opportunity for an individual sponsor to provide insight and commentary from their point-of-view directly to our audience. The editorial team does not participate in the writing or editing of SponsoredPosts.</p><hr/><a class=\"close-bar\" href=\"#\">Close</i></a></div>";
		 }
		 $("body").append(sponsorship_txt);
		 $(".about-sponsorships").css("top",event.pageY);
		 if ($("body").width() >= 955) { // need different behavior at desktop breakpoint and above
			 //find difference between X position clicked and body width and set as right attr 
			 //this is so tooltip doesn't overflow window
			 var xpos = $("body").width() - event.pageX;
			 $(".about-sponsorships").css({"right":xpos,"margin-right":"0"});
		 }
		 $(".about-sponsorships .close-bar").click(function() {
			 $(".about-sponsorships").remove();
			 return false;
		 });
		 return false;
	 });
	 
	 $('#insider-selector .label').click(function(e){
		    e.preventDefault();
		    e.stopPropagation();
		    $(this).parent().toggleClass('open');
		    $(this).parent().parent().toggleClass('open');
			            
          /*  $(window).scroll(function(){
				var pageScrollTop = $(window).scrollTop();
				var searchOffset = $('.fixed-bar').offset();				
				var searchOffsetTop = searchOffset.top;
				var searchHeight =  $('.fixed-bar').outerHeight();
				var fixedSearchHeight = $('.fixed-bar .fixed-search-wrapper').outerHeight();
				searchOffsetTop = searchOffsetTop + (searchHeight - fixedSearchHeight); //this handles where there's an intro blurb or not
			
				if (pageScrollTop > searchOffsetTop) {
					$('body').addClass('fixed-search-scroll');
					$('.fixed-bar').css('padding-bottom', fixedSearchHeight);
				}
				else {
					$('body').removeClass('fixed-search-scroll');
					$('.fixed-bar').css('padding-bottom', 0);
				}
			});*/
		    
		});

	//DealPost tooltip
	$(".dealpost-link").hover(function() {
		$(".dealpost-tooltip-outer").toggle();
	});
	 
	 // Tooltip click events for Contributor Network badges
	 $(".cn-click-tooltip").click(function(event) {
			
			$(".about-cn-users").remove(); //clean up any stray tooltips that may be hanging out
			
			var tooltipBegin = "<div class=\"about-cn-users\"><img class=\"arrow-up\" src=\"//idge.staticworld.net/nww/arrow-up.png\" alt=\"arrow up\" />";
			var tooltipEnd = "<hr/><a class=\"close-bar\" href=\"#\">Close</a></div>";
			var tooltipText = "";
			
			if ($(this).hasClass('advisor')) {
				tooltipText = "<p>This person has reached <strong>Advisor</strong> status as part of IDG's contributor Network.</p>";
			}
			else if ($(this).hasClass('influencer')) {
				tooltipText = "<p>This person has reached <strong>Influencer</strong> status as part of IDG's contributor Network.</p>";
			}
			
			else if ($(this).hasClass('thought-leader')) {
				tooltipText = "<p>This person has reached <strong>Thought Leader</strong> status as part of IDG's contributor Network.</p>";
			}
			
			else if ($(this).hasClass('cn-tooltip')) {
				tooltipText = "<p>The IDG Contributor Network is the best way to voice your opinion on our site. <br /><a class=\"cn-learn-more\" href=\"/contributor-network/signup.html\">Apply to be a contributor</a></p>";
			}
			 var user_txt = tooltipBegin + tooltipText + tooltipEnd;
			 $("body").append(user_txt);
			 $(".about-cn-users").css("top",event.pageY);
			 if ($("body").width() >= 955) { // need different behavior at desktop breakpoint and above
				 //find difference between X position clicked and body width and set as right attr 
				 //this is so tooltip doesn't overflow window
				 //adding 30px so that the tooltip point lands where the click event happened
				 var xpos = $("body").width() - event.pageX + 30;
				 $(".about-cn-users").css({"right":xpos,"margin-right":"-290px"});
			 }
			 $(".about-cn-users .close-bar").click(function() {
				 $(".about-cn-users").remove();
				 return false;
			 });
			 return false;
		 });
	 
	 //new functionality for sticky searchbox
	 if ($('.fixed-bar .fixed-search-wrapper').length > 0) {
			$(window).scroll(function(){
				var pageScrollTop = $(window).scrollTop();
				var searchOffset = $('.fixed-bar').offset();				
				var searchOffsetTop = searchOffset.top;
				var searchHeight =  $('.fixed-bar').outerHeight();
				var fixedSearchHeight = $('.fixed-bar .fixed-search-wrapper').outerHeight();
				searchOffsetTop = searchOffsetTop + (searchHeight - fixedSearchHeight); //this handles where there's an intro blurb or not
			
				if (pageScrollTop > searchOffsetTop) {
					$('body').addClass('fixed-search-scroll');
					$('.fixed-bar').css('padding-bottom', fixedSearchHeight);
				}
				else {
					$('body').removeClass('fixed-search-scroll');
					$('.fixed-bar').css('padding-bottom', 0);
				}
			});
	 }
	 
	 //learn more in expanded search bar intro text
	 $('#search-intro .more').click(function(){
		 $(this).addClass('open');
		 $('#search-intro .more-text').addClass('open');
	 });
	 
	//initialize any generic accordions that may be in use
	 if ($(".accordion").length > 0) {
		 initAccordion($(".accordion"));
	 }
});

//new functionality for sites with logins
$("li.signed-in").on("click", ".user-info .avatar, .user-info .user-name", function(e){
	 e.preventDefault();
	 e.stopPropagation();
	 $(".user-info .user-actions").toggleClass("open");
});


/* js functionality for the search page, static page nav */

$(document).ready(function(){
	 $('.filter-list').change( function() {
	    window.location.href = $('option:selected',this).data('jumpurl');
	 }); 
});



//************ jQuery Plug-in ************//
jQuery.fn.center = function () {
    this.css("position","absolute");
    this.css("top", (($(window).height() - this.outerHeight()) / 2) + 
                                                $(window).scrollTop() + "px");
    this.css("left", (($(window).width() - this.outerWidth()) / 2) + 
                                                $(window).scrollLeft() + "px");
    return this;
};

jQuery.fn.truncate = function (tLength) {
	if (typeof this.selector == 'string')  {
		if (tLength >= 3 && this.selector.length > 3) {
			return (this.selector.length > tLength)?this.selector.substr(0,tLength - 3) + "...":this.selector;
		} else {
			return this.selector;
		}
	};
};
function readCookie(name){
	return unescape(readRawCookie(name));
}

function readRawCookie(name){
	if(navigator.cookieEnabled&&document.cookie!=''){
		var strAll = document.cookie;
		var i1 = strAll.indexOf(name);
		if(i1!=-1){
			// skip name and '='
			i1 = i1+name.length+1;
			i2 = strAll.indexOf(';', i1);
			if(i2==-1) i2 = strAll.length;
			return strAll.substring(i1, i2);
		}
	}
	return "";
}

function isValidEmailAddress(emailAddress) {
    var pattern = new RegExp(/^[+a-zA-Z0-9._-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,5}$/i);
    return pattern.test(emailAddress);
}


/*--------------------------------------------------------
 * Name: getUrlParams
 * Description: Return URL parameters as a hash
 * Input: key - The key of the parameter for which you want the value. Returning all params (key is blank) is not supported currently.
 * Return: Hash of Parameters for 'key'
 * Usage: http://dev-narf4.fram.www.idgesg.infoworld.com/article/2608316/consumerization-of-it/the-proper-care-and-support-of-today-s-mobile-worker.html?myparam=%22%22&phint=newt%3Dcio_mobile&phint=idg_eid%3Db50de66d737b51d65b2ecb8faa838250#tk.CFOWORLDNLE_nlt_update_2016-12-06
 * 		getUrlParams('phint') - Returns {newt: "cio_mobile", idg_eid: "b50de66d737b51d65b2ecb8faa838250"
 * 		getUrlParams('myparam') - Returns {myparam: """"}
 *-------------------------------------------------------*/
function getUrlParams(key) {
	var map = new Object();
	var re = new RegExp('(?:\\?|&)' + key + '=(.*?)(?=&|$)','gi');
	while ((instance = re.exec(document.location.search)) != null) 
	{
		decoded = decodeURIComponent(instance[1]);
		if(decoded.indexOf('=') == -1) 
		{
			map[key] = decoded;
		}
		else 
		{
			var keyval = decoded.split('=');
			map[keyval[0]] = keyval[1];
		}
	}
	
	return map;
}
/* by default, $(".accordion") is passed in on ready(); can init others later if needed */
/* If you change this code, be sure to also change it in CSMB's thm_post.js */
function initAccordion($accordions) {
	$accordions.each(function() { //do this for each accordion passed in
		//console.log("Initializing "+$(this).attr("id"));
		var $toggles = $(this).find("[data-toggle='collapse']"); //gather the toggle controls for this accordion
		var hasActiveTagging = $toggles.hasClass("active"); //figure out if this accordion is using the "active" class to indicate an active heading
		
		//console.log($toggles);
		$toggles.each(function() { //do this for each toggle
			var currentTarget = $(this).data("target"); //what pane we wish to toggle, specified via data- attributes
			//console.log(currentTarget);
			var parent = $(this).data("parent"); //for 'true' accordion behavior, where every slide closes when another is opened
			//console.log(parent);
			var $notTarget = $(parent+" .collapse").not(currentTarget);
			$(this).click(function() { //bind click event for this toggle
				if (typeof parent !== "undefined") { //if 'true' accordion, first close everything but current target
					$notTarget.slideUp(); 
					$(parent).find($toggles).removeClass("active");
				}
				if (hasActiveTagging) {
					$(this).toggleClass("active");
				}
				$(currentTarget).slideToggle(); //then toggle target open/closed
			})
		});
	});
}
