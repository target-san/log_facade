#pragma once
#include <ostream>
#include "../util/FuncRef.hpp"
/*
    Logging macros - use these to write messages to log
*/

// Hard, unrecoverable error
#define $log_error(...) $log_error_at($LogCurrentChannel, $LogCurrentLocation, __VA_ARGS__)
// An error which can be possibly handled somewhere up the code hierarchy
#define $log_warn( ...) $log_warn_at( $LogCurrentChannel, $LogCurrentLocation, __VA_ARGS__)
// Informational message
#define $log_info( ...) $log_info_at( $LogCurrentChannel, $LogCurrentLocation, __VA_ARGS__)
// Debug data, like state of some structure after operation
#define $log_debug(...) $log_debug_at($LogCurrentChannel, $LogCurrentLocation, __VA_ARGS__)
// Highly-detailed tracing message - function enter/leave, exception being wrapped with additional context etc.
#define $log_trace(...) $log_trace_at($LogCurrentChannel, $LogCurrentLocation, __VA_ARGS__)

/** Establish named log channel till the end of current translation unit
    Only string literals can be used
    Such 'channel' may be used to filter logging messages
    
    @param  $identifier String literal which denotes current logging channel
*/
#define $LogChannel($identifier) static constexpr ::log_facade::Channel __log_facade_get_channel__(::log_facade::impl::AdlTag) { return ($identifier); }
/**
    Logging macros which are specialized by severity but allow to specify custom target and location
    May be used in cases where logging macro is invoked through some intermediate code,
    and what matters is the invocation site of that code
    
    @param[in] $target      Target name
    @param[in] $location    File and line where logging happens
*/
#define $log_error_at($channel, $location, ...) $log_perform_write(::log_facade::Severity::Error,   $channel, $location, __VA_ARGS__)
#define $log_warn_at( $channel, $location, ...) $log_perform_write(::log_facade::Severity::Warning, $channel, $location, __VA_ARGS__)
#define $log_info_at( $channel, $location, ...) $log_perform_write(::log_facade::Severity::Info,    $channel, $location, __VA_ARGS__)
/*
    Two lowest levels of logging are compiled-in only in debug mode or if explicitly enabled via macro
*/
#if (defined _DEBUG) || (defined LOG_DETAILED)
#   define $log_debug_at($channel, $location, ...) $log_perform_write(::log_facade::Severity::Debug, $channel, $location, __VA_ARGS__)
#   define $log_trace_at($channel, $location, ...) $log_perform_write(::log_facade::Severity::Trace, $channel, $location, __VA_ARGS__)
#else
#   define $log_debug_at($channel, $location, ...) (void())
#   define $log_trace_at($channel, $location, ...) (void())
#endif
/** The most explicit log writing macro. Does not infer any info from its current context
    
    @param[in] $severity    log severity level
    @param[in] $channel     log channel, defined by application
    @param[in] $location    file and line which should be used in log message as location
    @param[in] $first       First object to write, usually string; just ensures there's anything to write
*/
#define $log_perform_write($severity, $channel, $location, $first, ...) ( \
    ::log_facade::impl::is_enabled($severity, $channel, $location) \
        ? ::log_facade::impl::write($severity, $channel, $location, $log_format($first, __VA_ARGS__)) \
        : (void()) )

/** Substitutes with current 'channel' defined in current scope
*/
#define $LogCurrentChannel (__log_facade_get_channel__(::log_facade::impl::AdlTag {}))
/** Substitutes with current loation object, which contains current file and line
*/
#define $LogCurrentLocation (::log_facade::Location { __FILE__, __LINE__, __FUNC__ })
/** Defines default log formatting method, which simply dumps all specified expressions to provided stream
    Any override must be a macro which accepts variadic number of arguments and generates callable
    which accepts `std::ostream&` and returns nothing
*/
#ifndef $log_format
//  Contains implementation of defaultFormat which is a bit complicated to be shown here
#   include "DefaultFmt.hpp"
/** @brief Default log formatting method
*/
#   define $log_format(...) (::log_facade::impl::default_format(__VA_ARGS__))
#endif

namespace log_facade
{
// Importance level of log message
enum class Severity
{
    // In fact specifies no need for logging
    None,
    // Normal logging levels
    Error,
    Warning,
    Info,
    Debug,
    Trace,
    // Upper limit for the value of logging level
    _Count
};
/** @brief Defines log location
*/
struct Location
{
    const char* file;
    int         line;
    const char* func;
};

using Channel       = const char*;
using WriterFunc    = util::FuncRef<void(std::ostream&)>;

namespace impl
{
/**
    Checks if logging is enabled for provided severity level, channel identifier and location

    @param  severity    Logging level
    @param  channel     A string which identifies log invocation context; meaning is implementation-defined
    @param  location    Logging message location in sources
    @return             true if message should be writter, false otherwise
*/
bool is_enabled(Severity severity, Channel channel, Location location);
/** @brief Deliver message to logging subsystem
  
    Writes specified message with specified metadata to log
    Not guaranteed to check if log is enabled for specified severity and target
    
    @param  severity    Logging level
    @param  channle     A string which identifies log invocation context; meaning is implementation-defined
    @param  location    File name and line number where logging happens
    @param  writer      Function which receives stream and writes logging message into it
*/
void write(Severity severity, Channel channel, Location location, WriterFunc writer);
/// Enables ADL-based deduction on which "log channel" function to use
struct AdlTag {};
// Returns default log channel, empty string in our case
static inline Channel __log_facade_get_channel__(...)
{
    return "";
}

} // namespace impl

} // namespace log_facade
