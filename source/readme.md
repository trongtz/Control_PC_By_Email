# ỨNG DỤNG ĐIỀU KHIỂN MÁY TÍNH TỪ XA BẰNG GMAIL

Ứng dụng này cung cấp khả năng điều khiển máy tính từ xa thông qua Gmail. Người dùng có thể gửi các lệnh điều khiển từ một tài khoản Gmail được cấp quyền để thực hiện các thao tác trên máy tính từ xa. 

---

## Cách thức hoạt động

1. Người dùng gửi lệnh điều khiển đến tài khoản Gmail Client của ứng dụng (người gửi sử dung email đã được cấp quyền)
2. Client nhận email, phân tích nội dung, và chuyển tín hiệu điều khiển đến Server.  
3. Server thực thi lệnh trên máy tính bị điều khiển và gửi kết quả phản hồi lại Client.  
4. Client đóng gói kết quả và gửi trả lại cho người dùng qua email.

---

## Hướng dẫn tải và sử dụng ứng dụng từ mã nguồn
Chú ý:  Trong zip sẽ có 2 file zip khác: source code (mã nguồn hoàn chỉnh) và remote_control_application (folder chứa file thực thi ứng dụng, chạy ngay mà không cần IDE).

1. Tải file `23120100-23120106-23120177.zip` và giải nén vào một thư mục trên máy tính của bạn.  
2. Kiểm tra xem các thành phần và công cụ cần thiết đã được cài đặt trên máy tính, bao gồm:  
   - Visual Studio (phiên bản mới nhất).  
   - Thư viện OpenCV.  
   - Thư viện EASendmail

---

## Hướng dẫn cấu hình và xây dựng ứng dụng

### 1. Cấu hình địa chỉ IP
- Vào file ip_address.txt và nhập địa chỉ IP của máy tính cần điều khiển (nên thực hiện bước này trước khi chạy client.exe)

### 2. Gắn các tệp hỗ trợ (chỉ thực hiện khi chương trình báo lỗi với thư viện EASendmail)
- Vào thư mục program của EASendmailđã cài đặt trước đó (ví dụ "C:\Program Files (x86)\EASendMail") -> include -> tlh
- Sao chép hai tệp `easendmailobj.tlh` và `easendmailobj.tli` từ thư mục này vào thư mục `client` (thư mục nằm trong dự án)

---

## Khởi chạy ứng dụng

1. Điều hướng đến thư mục chứa mã nguồn đã được giải nén:  
   - Đối với Client:  
     `socket > client > x64 > Debug > client.exe`.  
   - Đối với Server:  
     `socket > server > x64 > Debug > server.exe`.  

2. Khởi chạy cả hai tệp `client.exe` và `server.exe` để bắt đầu sử dụng ứng dụng.

---

## Lưu ý quan trọng

- Đảm bảo rằng địa chỉ IP trong file ip_address.txt đã được điền trước khi chạy client.exe
- Trước khi gửi lệnh, hãy đăng kí quyền điều khiển máy bằng cách gửi email với nội dung "add thitkhomamruot7749"
- Nếu có bất kỳ lỗi nào trong quá trình sử dụng, hãy kiểm tra báo cáo lỗi hoặc xem lại phần hướng dẫn trong tài liệu đính kèm.  
- Tài khoản Gmail sử dụng cần được kích hoạt chế độ "Less Secure App" (nếu cần thiết) để cho phép ứng dụng truy cập.

---

## Tác giả

- Tên nhóm/phát triển: [Hàn Vũ Phương Uyên, Trần Minh Trọng, Phạm Hương Trà].  
- Liên hệ: [thitkhomamruot7749@gmail.com].  

