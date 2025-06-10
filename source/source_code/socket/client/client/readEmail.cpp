#include <curl/curl.h>
#include <iostream>
#include <string>
#include<fstream>

// Hàm callback để ghi dữ liệu nhận được vào chuỗi
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userData) {
    size_t totalSize = size * nmemb;
    userData->append((char*)contents, totalSize);
    return totalSize;
}

int countEmailsInGmail(std::string username, std::string password) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    int emailCount = 0;

    // Khởi tạo libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // Cấu hình IMAP URL (kết nối tới server IMAP của Gmail)
        std::string url = "imaps://imap.gmail.com:993/INBOX";

        // Thiết lập các thông số yêu cầu
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);  // Sử dụng SSL/TLS

        // Cấu hình chế độ IMAP SEARCH để đếm số lượng email
        std::string searchCmd = "SEARCH ALL";  // Lệnh tìm tất cả các email
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, searchCmd.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Gửi yêu cầu và nhận dữ liệu
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return -1;  // Nếu có lỗi, trả về -1
        }

        // Xử lý phản hồi (readBuffer chứa các ID email trả về từ server)
        size_t pos = 0;
        while ((pos = readBuffer.find(" ", pos)) != std::string::npos) {
            emailCount++;
            pos++;
        }

        // Dọn dẹp
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return emailCount;  // Trả về số lượng email trong INBOX
}

std::string getEmailInfo(const std::string& rawEmail) {
    std::string senderEmail;
    std::string body;
    size_t startPos, endPos;

    // Extract Sender (From)
    startPos = rawEmail.find("From:");
    if (startPos != std::string::npos) {
        startPos += 5; // Skip "From:"
        endPos = rawEmail.find("\r\n", startPos);
        if (endPos != std::string::npos) {
            senderEmail = rawEmail.substr(startPos, endPos - startPos);
            // Trim leading/trailing spaces
            senderEmail.erase(0, senderEmail.find_first_not_of(" \t"));
            senderEmail.erase(senderEmail.find_last_not_of(" \t") + 1);
            // If the email is in the form "Name <email@example.com>", extract only the email
            size_t emailStart = senderEmail.find("<");
            size_t emailEnd = senderEmail.find(">");
            if (emailStart != std::string::npos && emailEnd != std::string::npos) {
                senderEmail = senderEmail.substr(emailStart + 1, emailEnd - emailStart - 1);
            }
        }
    }

    // Extract Body (plain text preferred, fallback to HTML)
    // Try to find the plain text body (Content-Type: text/plain)
    startPos = rawEmail.find("Content-Type: text/plain");
    if (startPos != std::string::npos) {
        startPos = rawEmail.find("\r\n\r\n", startPos);
        if (startPos != std::string::npos) {
            startPos += 4; // Skip the CRLF
            endPos = rawEmail.find("\r\n", startPos);
            if (endPos != std::string::npos) {
                body = rawEmail.substr(startPos, endPos - startPos);
            }
            else {
                body = rawEmail.substr(startPos);
            }
        }
    }

    // If no plain text body, try to find HTML body (Content-Type: text/html)
    if (body.empty()) {
        startPos = rawEmail.find("Content-Type: text/html");
        if (startPos != std::string::npos) {
            startPos = rawEmail.find("\r\n\r\n", startPos);
            if (startPos != std::string::npos) {
                startPos += 4; // Skip the CRLF
                endPos = rawEmail.find("\r\n--", startPos);
                if (endPos != std::string::npos) {
                    body = rawEmail.substr(startPos, endPos - startPos);
                }
                else {
                    body = rawEmail.substr(startPos);
                }
            }
        }
    }

    // Combine sender email and body with a separator
    return senderEmail + "-" + body;
}

std::string readEmail(const std::string& username, const std::string& appPassword, int index) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    std::string url = "imaps://imap.gmail.com:993/INBOX/;MAILINDEX=" + std::to_string(index);

    curl = curl_easy_init();
    if (curl) {
        // Cấu hình thông tin đăng nhập
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, appPassword.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Cấu hình callback để ghi dữ liệu vào chuỗi `readBuffer`
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Sử dụng SSL vì Gmail yêu cầu
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

        // Kết nối và nhận dữ liệu
        res = curl_easy_perform(curl);

        // Kiểm tra kết quả và xử lý lỗi nếu có
        if (res != CURLE_OK) {
            std::cerr << "Lỗi khi kết nối: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            // Get the body of the email
            std::string emailBody = getEmailInfo(readBuffer);
            return emailBody;
        }
        // Dọn dẹp CURL
        curl_easy_cleanup(curl);
    }

    return "";
}