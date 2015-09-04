<%
	filesize=Request.TotalBytes '获得上传文件的大小  
	filedata=Request.BinaryRead(filesize) '获得上传文件的二进制数据  
	Response.Write filesize '在浏览器上显示二进制数据  
%>