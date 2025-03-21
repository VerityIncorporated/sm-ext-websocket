// HTTP FUNCTIONS ARE NOT COMPLETED, THEY ARE STILL BEING TESTED

// Define a typeset for HTTP response callbacks
typeset ResponseCallback
{
  /**
  * Function called when an HTTP response is received
  *
  * @param http         HTTP request object
  * @param body         Response body
  * @param statusCode   HTTP status code
  * @param bodySize     Size of the response body
  */
  function void (HttpRequest http, const char[] body, int statusCode, int bodySize);
}

// Define a methodmap for HttpRequest, which extends Handle
methodmap HttpRequest < Handle
{
  /**
  * Create a new HTTP request
  *
  * @param url      Request URL
  * @param method   HTTP method (default is "GET")
  */
  public native HttpRequest(const char[] url, const char[] method = "GET");

  /**
  * Set the callback for when an HTTP response is received
  *
  * @param fResponse    Function to call when an HTTP response is received
  */
  public native void SetResponseCallback(ResponseCallback fResponse);

  /**
  * Perform the HTTP request
  */
  public native void Perform();

  /**
  * Set the body of the HTTP request
  *
  * @param body   Body of the request
  */
  public native void SetBody(const char[] body);

  /**
  * Add a header to the HTTP request
  *
  * @param key   Header key
  * @param value Header value
  */
  public native void AddHeader(const char[] key, const char[] value);

  /**
  * Get a header from the HTTP request
  *
  * @param key   Header key
  * @param value Buffer to store the header value
  * @param maxLength Maximum length of the value buffer
  * @return True if the header was found, false otherwise
  */
  public native bool GetHeader(const char[] key, char[] value, int maxLength);

  /**
  * Check if a header exists in the HTTP request
  *
  * @param key   Header key
  * @return True if the header exists, false otherwise
  */
  public native bool HasHeader(const char[] key);
}