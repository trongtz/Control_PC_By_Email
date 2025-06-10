#include "lib.h" 

string htmlTop =
"<!DOCTYPE html>"
"<html lang=\"en\">"
"<head>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
"<title>Styled Command Result</title>"
"<style>"
"body {"
"    font-family: 'Arial', sans-serif;"
"    background-color: #f4f4f9;"
"    color: #333;"
"    padding: 20px;"
"    margin: 0;"
"}"
".container {"
"    max-width: 800px;"
"    margin: 0 auto;"
"    background-color: #fff;"
"    padding: 20px;"
"    border-radius: 8px;"
"    box-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);"
"}"
"h1 {"
"    color: #4CAF50;"
"    font-size: 24px;"
"    text-align: center;"
"    margin-bottom: 20px;"
"}"
"p {"
"    font-size: 18px;"
"    line-height: 1.6;"
"    margin: 10px 0;"
"}"
".result {"
"    background-color: #e9f7f1;"
"    padding: 10px;"
"    border-left: 5px solid #4CAF50;"
"    margin-top: 10px;"
"}"
"table {"
"    width: 100%;"
"    margin-top: 20px;"
"    border-collapse: collapse;"
"}"
"table, th, td {"
"    border: 1px solid #ddd;"
"}"
"th, td {"
"    padding: 12px;"
"    text-align: left;"
"}"
"th {"
"    background-color: #4CAF50;"
"    color: white;"
"}"
"tr:nth-child(even) {"
"    background-color: #f2f2f2;"
"}"
"</style>"
"</head>"
"<body>"
"<div class=\"container\">"
"<h1>Remote control result: </h1>"
"<div class=\"result\">";

string htmlBottom =
"</div>"
"</div>"
"</body>"
"</html>";

string htmlBoard =
"<table>"
"<thead>"
"<tr>"
"<th>Index</th>"
"<th>Program</th>"
"</tr>"
"</thead>"
"<tbody>";

string enterInHTML(string response) {
	int i = 0;
	while (i < response.length()) {
		if (response[i] == '\n') {
			string tmp1 = response.substr(0, i + 1);
			string tmp2 = response.substr(i + 1, response.length() - i - 1);
			response = tmp1 + "<br>" + tmp2;
			i += 4;
		}
		i++;
	}
	return response;
}

string customMailDisplay(string response) {
	string htmlBody = "";
	response = enterInHTML(response);
	htmlBody = "<p><strong>" + response + "</strong></p>";

	return htmlTop + htmlBody + htmlBottom;
}