var ticker = "URKA  000.00 D";
var ticker_london = "LOND   00.00 D";

// Set callback for the app ready event
Pebble.addEventListener("ready",
                        function(e) {
                          console.log("js connected!" + e.ready);
                          console.log(e.type);
							sendTicker();
							fetchTicker();
                        });

// Set callback for appmessage events
Pebble.addEventListener("appmessage",
                        function(e) {
                          console.log("message");
							if (e.payload.fetch) {
								console.log('fetch msg');
								fetchTicker();
							} else {
								console.log('unknown msg');
							}
                        });


function sendTicker() {
	Pebble.sendAppMessage({"price": ticker},
		function (e) {
			console.log("Trans: " + e.data.transactionId);
		},
		function (e) {
			console.log("Send err: " + e.error.message);
			console.log("Sending again...");
			sendTicker();
		}
	);
}

function sendTickerLondon() {
	Pebble.sendAppMessage({"price_london": ticker_london},
		function (e) {
			console.log("Trans: " + e.data.transactionId);
		},
		function (e) {
			console.log("Send err: " + e.error.message);
			console.log("Sending again...");
			sendTickerLondon();
		}
	);
}

///<tbody>\n<tr class="odd">\n<td>(.*?)<\/td>/i.exec()
///class="datetime large">(.|\r|\n)*?img src=.*?>(.*?)</i.exec()

function fetchTicker() {
  // fetch price from moscow stock exchange
	var req = new XMLHttpRequest();
  // build the GET request
	req.open('GET', "https://www.google.com/finance?cid=175655662465721", true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      // 200 - HTTP OK
      if(req.status == 200) {
        console.log("got the page");
				var match = /ref_175655662465721_l">(.*?)</i.exec(req.responseText);
				console.log("found match: "+match.length);
				if (match.length > 1 && match[1].length == 6) {
					console.log("got correct price format");
					ticker = "URKA  " + match[1] + " ";
					match = /ref_175655662465721_c">(.*?)</i.exec(req.responseText);
					if (match.length > 1 && match[1].length > 1 &&
						(match[1][0] == "+" || match[1][0] == "-")) {
						console.log("got correct price change format");
						if (match[1][0] == "+") {
							ticker += "U";
						} else
						if (match[1][0] == "-") {
							ticker += "D";
						} else {
							ticker += " ";
						}
					}
					console.log("sending new ticker to pebble");
					sendTicker();
				} else {
					console.log("incorrect length for price detected");
				}
      } else {
        console.log("Request returned error code " + req.status.toString());
      }
    }
  };
  req.send(null);
	
	// fetch price from london stock exchange
	var req_l = new XMLHttpRequest();
  // build the GET request
	//req_l.open('GET', "http://www.londonstockexchange.com/exchange/prices-and-markets/stocks/summary/company-summary.html?fourWayKey=US91688E2063USUSDIOBE", true);
	req_l.open('GET', "http://m.londonstockexchange.com/exchange/mobile/stocks/summary.html?fourWayKey=US91688E2063USUSDIOBE", true); // mobile version
	//req_l.setRequestHeader("User-Agent", "Mozilla/5.0 (X11; Linux i686) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/40.0.2214.91 Safari/537.36");
  req_l.onload = function(e) {
    if (req_l.readyState == 4) {
      // 200 - HTTP OK
      if(req_l.status == 200) {
				console.log("lon: got the london page");
				//var match = /<tbody>(.|\r|\n)*?<tr class="odd">(.|\r|\n)*?<td>(.*?)</i.exec(req_l.responseText);
				var match = /class="tr darkEven(.|\r|\n)*?span>(.*?)</i.exec(req_l.responseText); // mobile version
				console.log("lon: found match: "+match.length);
				if (match.length > 1 && match[2].length == 5) {
					console.log("lon: got correct price format");
					var price = parseFloat(match[2]);
					if (price >= 20) {
						ticker_london = "LOND   " + match[2] + " ";
					} else {
						ticker_london = "LOND    " + match[2] + " ";
					}
					//match = /class="datetime large">(.|\r|\n)*?img src=.*?>(.*?)</i.exec(req_l.responseText);
					match = /class="tr darkEven(.|\r|\n)*?span>(.*?)<(.|\r|\n)*?color.*?>(.|\r|\n)*?(.*?)</i.exec(req_l.responseText); // mobile version
					if (match.length > 1 && match[5].length > 1 &&
						(match[5][0] == "+" || match[5][0] == "-")) {
						console.log("lon: got correct price change format");
						if (match[5][0] == "+") {
							ticker_london += "U";
						} else
						if (match[5][0] == "-") {
							ticker_london += "D";
						} else {
							ticker_london += " ";
						}
					}
					console.log("lon: sending new london ticker to pebble");
					sendTickerLondon();
				} else {
					console.log("lon: incorrect length for price detected");
				}
      } else {
				console.log("lon: Request returned error code " + req_l.status.toString());
      }
    }
  };
  req_l.send(null);
	
}