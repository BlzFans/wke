<%
	filesize=Request.TotalBytes '����ϴ��ļ��Ĵ�С  
	filedata=Request.BinaryRead(filesize) '����ϴ��ļ��Ķ���������  
	Response.Write filesize '�����������ʾ����������  
%>