extern "C" {

extern long curl_cookies_count(void *curl);
extern void* curl_first_cookie(void *curl);
extern void* curl_next_cookie(void *cookie);
extern const char* curl_cookie_name(void *cookie);
extern const char* curl_cookie_value(void *cookie);
extern const char* curl_cookie_domain(void *cookie);
extern const char* curl_cookie_path(void *cookie);
extern bool curl_cookie_secure(void *cookie);
extern int64_t curl_cookie_expires(void *c);
extern void* curl_cookie_add(void *curl, const char* value, const char* domain, const char* path);

}

class HttpCookie
{
public:
    HttpCookie()
    {
        reset();
    }

    void reset()
    {
        expires_ = 0;
        domain_ = String();
        path_ = String();
        name_ = String();
        value_ = String();
        secure_ = false;
        httpOnly_ = false;
    }
    
    bool isSecure()
    {
        return secure_;
    }

    void setSecure(bool enable)
    {
        secure_ = enable;
    }

    bool isHttpOnly() const
    {
        return httpOnly_;
    }

    void setHttpOnly(bool enable)
    {
        httpOnly_ = enable;
    }

    bool isSessionCookie() const
    {
        return expires_ == 0;
    }

    int64_t expires() const
    {
        return expires_;
    }

    void setExpires(int64_t expires)
    {
        expires_ = expires;
    }

    const String& domain() const
    {
        return domain_;
    }

    void setDomain(const String& domain)
    {
        if (!domain.isEmpty() && domain[0] != L'.')
        {
            domain_ = String(L".") + domain;
            return;
        }

        domain_ = domain;
    }

    const String& path() const
    {
        return path_;
    }
	
    void setPath(const String& path)
    {
        path_ = path;
    }
	
    const String& name() const
    {
        return name_;
    }

    void setName(const String& name)
    {
        name_ = name;
    }

    const String& value() const
    {
        return value_;
    }

    void setValue(const String& value)
    {
        value_ = value;
    }

private:
    int64_t expires_;
    String domain_;
    String path_;
    String name_;
    String value_;
    bool secure_;
    bool httpOnly_;
};

inline bool isParentDomain(const String& domain, const String& reference)
{
    if (reference.isEmpty())
        return reference.isEmpty();

    if (domain.endsWith(reference))
        return true;

    if (domain.length() + 1 == reference.length())
    {
        return reference.endsWith(domain);
    }

    return false;
}

inline bool isParentPath(const String& path, const String& reference)
{
    return path.startsWith(reference);
}

class CookieJar
{
public:
    void set(void* curl)
    {
        void* cookie = curl_first_cookie(curl);
        while (cookie)
        {
            HttpCookie c;
            c.setName(curl_cookie_name(cookie));
            c.setValue(curl_cookie_value(cookie));
            c.setDomain(curl_cookie_domain(cookie));
            c.setPath(curl_cookie_path(cookie));
            add(c);
            
            cookie = curl_next_cookie(cookie);
        }
    }
    
    void set(void* handle, const WebCore::KURL& url, const String& value)
    {
        String host = url.host();
        String path = url.path();
        
        if (handle)
        {
            void* cookie = curl_cookie_add(handle, value.utf8().data(), host.utf8().data(), path.utf8().data());
            if (cookie)
            {
                HttpCookie c;
                c.setName(curl_cookie_name(cookie));
                c.setValue(curl_cookie_value(cookie));
                c.setDomain(curl_cookie_domain(cookie));
                c.setPath(curl_cookie_path(cookie));
                add(c);
                return;
            }
        }

        size_t pos = value.find(L'=');
        if (pos != notFound)
        {
            size_t end = value.find(L';');
            if (end == notFound)
                end = value.length();

            HttpCookie c;
            c.setName(value.substring(0, pos));
            c.setValue(value.substring(pos + 1, end - pos - 1));
            c.setDomain(host);
            c.setPath(path);
            add(c);
        }
    }
    
    String get(const WebCore::KURL& url)
    {
        Vector<HttpCookie> cookies;
        String domain = url.host();
        String path = url.path();

        for (unsigned int i = 0; i < cookies_.size(); ++i) {
            if (!isParentDomain(domain, cookies_[i].domain()))
                continue;

            if (!isParentPath(path, cookies_[i].path()))
                continue;
                
            cookies.append(cookies_[i]);
        }
        
        String res;
        for (unsigned int i = 0; i < cookies.size(); ++i)
        {
            if (i > 0)
                res += L"; ";
                
            res += cookies[i].name() + L"=" + cookies[i].value();
        }
        
        return res;
    }
    
    void add(const HttpCookie& c)
    {
        for (unsigned int i = 0; i < cookies_.size(); ++i)
        {
            if (cookies_[i].name() == c.name() &&
                cookies_[i].domain() == c.domain() &&
                cookies_[i].path() == c.path())
            {
                cookies_[i] = c;
                return;
            }
        }

        cookies_.append(c);
    }

private:
    Vector<HttpCookie> cookies_;
};

extern CookieJar cookieJar;