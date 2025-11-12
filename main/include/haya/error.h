/**
 * @file haya/error.h
 * @brief Defines the standard error codes used throughout the Haya library.
 *
 * This file uses an X-Macro pattern to define the HyErr enumeration and a
 * corresponding function (hyErrToStr) to convert error codes into
 * human-readable strings.
 */

#ifndef __HAYA_ERROR_H
#define __HAYA_ERROR_H

/**
 * @def _HY_ERR_LIST
 * @brief An internal X-Macro list used to generate error enums and strings.
 * @internal
 *
 * This macro defines all the error codes in one place. It is processed
 * multiple times to build the HyErr enum and the _HY_ERR_STRINGS array.
 */
#define _HY_ERR_LIST        \
    X(HY_ERR_NONE)          \
    X(HY_ERR_BAD_ARGS)      \
    X(HY_ERR_MALLOC_FAILED) \
    X(HY_ERR_FAILURE)       \
    X(HY_ERR_TIMEOUT)

/**
 * @brief Internal string array for error code lookup.
 * @internal
 */
extern const char *_HY_ERR_STRINGS[];

/**
 * @brief Enumeration of standard Haya library error codes.
 */
typedef enum
{
#define X(name) name,
    /** @brief Operation completed successfully. */
    HY_ERR_NONE,
    /** @brief Invalid arguments were provided to a function. */
    HY_ERR_BAD_ARGS,
    /** @brief A memory allocation (e.g., malloc) failed. */
    HY_ERR_MALLOC_FAILED,
    /** @brief A generic, unspecified failure occurred. */
    HY_ERR_FAILURE,
    /** @brief An operation did not complete within the expected time. */
    HY_ERR_TIMEOUT,
#undef X
    _HY_ERR_END /**< @internal Marker for the end of the enum. */
} HyErr;

/**
 * @brief Converts a Haya error code into a human-readable string.
 *
 * @param err The error code (HyErr) to convert.
 * @return A constant string literal representing the error name (e.g., "HY_ERR_NONE").
 * Returns "UNKNOWN" if the error code is invalid.
 */
const char *hyErrToStr(HyErr err);

#endif