/** @file
 * @brief Functions here are assigned (at initialisation) to callback function variables 
 * from where they are called (by scheduler.cpp).
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
static inline void openInjector(void)   { openInjector(channel); }

/**
 * @brief Callback that closes one injector.
 * 
 * Example:
 *     closeInjector<2>();
 * 
 * @tparam channel *One* based index of the injector to close
 */
template <uint8_t channel>
static inline void closeInjector(void)   { closeInjector(channel); }
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
static inline void beginCoilCharge(void)   { beginCoilCharge(channel); }

/**
 * @brief Callback that ends coil charging & fires a spark.
 * 
 * Example:
 *     endCoilCharge<2>();
 * 
 * @tparam channel *One* based index of the coil to fire
 */
template <uint8_t channel>
static inline void endCoilCharge(void)   { endCoilCharge(channel); }

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

