#ifndef __HTTP_SERVER_HTTP_SERVER_H
#define __HTTP_SERVER_HTTP_SERVER_H

#include "esp_err.h"
#include "esp_http_server.h"

/**
 * @name Embedded Resources
 * @brief External linker symbols for embedded binary data (HTML).
 */
///@{
extern const uint8_t _404_html_start[] asm("_binary_404_min_html_start"); /*!< @brief Start address of the embedded 404 HTML file. */
extern const uint8_t _404_html_end[] asm("_binary_404_min_html_end");     /*!< @brief End address of the embedded 404 HTML file. */
///@}

/**
 * @brief Initializes and starts the HTTP server.
 *
 * This function configures the HTTP server with the specified port,
 * registers the default error handlers (like the 404 page), and
 * starts the server instance.
 *
 * @param[out] server A pointer to the httpd_handle_t where the server handle will be stored.
 * @param[in]  port   The TCP port number to listen on (e.g., 80).
 *
 * @return
 * - ESP_OK: Server started successfully.
 * - ESP_FAIL: Failed to start the server.
 * - ESP_ERR_INVALID_ARG: Null pointer or invalid port provided.
 */
esp_err_t hyHttpServerSetup(httpd_handle_t *server, uint16_t port);

/**
 * @brief Default error handler callback for HTTP 404 (Not Found).
 *
 * This function is registered via httpd_register_err_handler(). It intercepts
 * requests to unknown URIs and serves the embedded custom 404 HTML page.
 *
 * @param req Pointer to the HTTP request structure.
 * @param err The HTTP error code (expected to be HTTPD_404_NOT_FOUND).
 *
 * @return
 * - ESP_OK: Response sent successfully.
 * - ESP_FAIL: Failed to send response.
 */
esp_err_t _hyHttpServer404(httpd_req_t *req, httpd_err_code_t err);

#endif