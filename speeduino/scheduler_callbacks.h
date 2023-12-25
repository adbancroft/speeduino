/** @file
 * @brief Provides the building block functions for ignition & injection scheduler callbacks.
 */

#pragma once

#include "scheduledIO.h"

/** @name Shared Callback Defintions */
///@{

/** @brief The type of the scheduler callback functions. */
typedef void (*voidVoidCallback)(void);

/** @brief A voidVoidCallback that does nothing. */
static inline void nullCallback(void) { return; }
///@}

///@cond
// Private to the callback implementations (below)
namespace scheduler_callback_detail {
    
    // Since we know both the channel index and maximum number of channels at compile 
    // time, we use this template to apply defensive checks at compile time rather than
    // runtime (for performance).
    // E.g. instead of a runtime boolean check:
    //      if (channel<INJ_CHANNELS)
    // we use call_if to avoid the function call altogether.

    // Base template, specialized below.
    template <bool> struct call_if {};

    // Template specialization: do not call the free function
    template <> struct call_if<false> {
        template <typename TFunction, typename...TArgs>
        // cppcheck-suppress misra-c2012-8.2
        static inline __attribute__((cold)) void call (TFunction, TArgs...){
            // Do nothing
        }
    };

    // Template specialization: call the supplied free function
    template <> struct call_if<true> {
        template <typename TFunction, typename...TArgs>
        static inline __attribute__((hot)) void call (TFunction f, TArgs...args){
            f(args...); 
        }
    };
}
///@endcond

/** @name Injector Callbacks */
///@{

/**
 * @brief Callback that opens one injector.
 * 
 * Example:
 *     openInjectors<1>();
 *  
 * @tparam channel *One* based index of the injector to open
 */
template <uint8_t channel>
static inline void openInjectorT(void)   { scheduler_callback_detail::call_if<channel<=INJ_CHANNELS>::call(openInjector, channel); }

/**
 * @brief Callback that closes one injector.
 * 
 * Example:
 *     closeInjector<2>();
 * 
 * @tparam channel *One* based index of the injector to close
 */
template <uint8_t channel>
static inline void closeInjectorT(void)   { scheduler_callback_detail::call_if<channel<=INJ_CHANNELS>::call(closeInjector, channel); }
///@}

/** @name Ignition Callbacks */
///@{

/**
 * @brief Callback that begins charging a coil.
 * 
 * Example:
 *     beginCoilCharge<1>();
 * 
 * @tparam channel *One* based index of the coil to charge
 */
template <uint8_t channel>
static inline void beginCoilChargeT(void)   { scheduler_callback_detail::call_if<channel<=IGN_CHANNELS>::call(beginCoilCharge, channel); }

/**
 * @brief Callback that ends coil charging & fires a spark.
 * 
 * Example:
 *     endCoilCharge<2>();
 * 
 * @tparam channel *One* based index of the coil to fire
 */
template <uint8_t channel>
static inline void endCoilChargeT(void)   { scheduler_callback_detail::call_if<channel<=IGN_CHANNELS>::call(endCoilCharge, channel); }

///@}

/** @name Callback chaining */
///@{
/// @cond 
// Base case - called but optimized out, just here to get the compiler to behave.
template<void* = nullptr>
static inline void compoundCallback(void) { }
/// @endcond

/**
 * @brief Callback that calls one or more other callbacks. Callback chain is defined at compile time.
 * 
 * Example:
 *     compoundCallback<endCoilCharge<1>, beginCoilCharge<2>>();
 * will sequentially end coil 1 charge & begin charging coil 2
 * 
 * @note This uses *compile time* recursion via a variadic template to capture
 * the callback chain with zero RAM overhead
 * 
 * @tparam firstCallback Initial callback
 * @tparam remainingCallbacks [optional] Subsequent callbacks to chain.
 */
template <voidVoidCallback firstCallback, voidVoidCallback... remainingCallbacks>
static inline void compoundCallback(void)   { firstCallback(); compoundCallback<remainingCallbacks...>(); }
///@}

