/*
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "CookieJar.h"

#include "Cookie.h"
#include "Document.h"
#include "KURL.h"
#include "PlatformString.h"
#include <wtf/HashMap.h>
#include <wtf/text/StringHash.h>

//wke++++++
#include "wkeCookieJar.h"
#include "DocumentLoader.h"
#include "MainResourceLoader.h"
#include "ResourceHandleInternal.h"
CookieJar cookieJar;
//wke++++++

namespace WebCore {

//wke++++++
//static HashMap<String, String> cookieJar;

CURL* curlHandle(const Document* document)
{
    if (document == NULL)
        return NULL;

    DocumentLoader* docLoader = document->loader();
    if (docLoader == NULL)
        return NULL;

    ResourceLoader* resLoader = docLoader->mainResourceLoader();
    if (resLoader == NULL)
        return NULL;

    ResourceHandle* handle = resLoader->handle();
    if (handle == NULL)
        return NULL;

    ResourceHandleInternal* d = handle->getInternal();
    if (d == NULL)
        return NULL;

    return d->m_handle;
}
//wke++++++

void setCookies(Document* document, const KURL& url, const String& value)
{
    //wke++++++
    CURL* handle = curlHandle(document);
    cookieJar.set(handle, url, value);
    //wke++++++
}

String cookies(const Document* document, const KURL& url)
{
    //wke++++++
    return cookieJar.get(url);
    //wke++++++
}

String cookieRequestHeaderFieldValue(const Document* /*document*/, const KURL& url)
{
    // FIXME: include HttpOnly cookie.
    //wke++++++
    return cookieJar.get(url);
    //wke++++++
}

bool cookiesEnabled(const Document* /*document*/)
{
    return true;
}

bool getRawCookies(const Document*, const KURL&, Vector<Cookie>& rawCookies)
{
    // FIXME: Not yet implemented
    rawCookies.clear();
    return false; // return true when implemented
}

void deleteCookie(const Document*, const KURL&, const String&)
{
    // FIXME: Not yet implemented
}

#if !PLATFORM(EFL)
void setCookieStoragePrivateBrowsingEnabled(bool enabled)
{
    // FIXME: Not yet implemented
}
#endif

void getHostnamesWithCookies(HashSet<String>& hostnames)
{
    // FIXME: Not yet implemented
}

void deleteCookiesForHostname(const String& hostname)
{
    // FIXME: Not yet implemented
}

void deleteAllCookies()
{
    // FIXME: Not yet implemented
}

}
