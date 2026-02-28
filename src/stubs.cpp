// just use if you need, in build this is not used

#include "generated/larecomp_init.h"
#include <rex/runtime/guest/function.h>
#include <rex/logging.h>

#include <cmath>

using namespace rex::runtime::guest;

extern "C" {
    // Rounding function used by the game
    float roundevenf(float x) {
        return std::rint(x);
    }

    // Memory Management Hooks (RtlAllocateHeap, RtlFreeHeap, etc.)
    // These will be linked to the implementations in heap.cpp
    extern dword_result_t RtlAllocateHeap_entry(dword_t hHeap, dword_t dwFlags, dword_t dwBytes);
    extern dword_result_t RtlFreeHeap_entry(dword_t hHeap, dword_t dwFlags, dword_t ptr);
    extern dword_result_t RtlSizeHeap_entry(dword_t hHeap, dword_t dwFlags, dword_t ptr);

    // RtlAllocateHeap (Ordinal 165)
    PPC_FUNC(rex_sub_826C5E00) {
        ctx.r3.u64 = RtlAllocateHeap_entry(ctx.r3.u32, ctx.r4.u32, ctx.r5.u32);
    }

    // RtlFreeHeap (Ordinal 166)
    PPC_FUNC(rex_sub_826C5E80) {
        ctx.r3.u64 = RtlFreeHeap_entry(ctx.r3.u32, ctx.r4.u32, ctx.r5.u32);
    }

    // RtlSizeHeap (Ordinal 167)
    PPC_FUNC(rex_sub_826C5F00) {
        ctx.r3.u64 = RtlSizeHeap_entry(ctx.r3.u32, ctx.r4.u32, ctx.r5.u32);
    }

    // Stubs for functions that cause issues during initialization
    // sub_82132740 and sub_82132820 are often related to static constructors
    // that might have invalid pointers in the recompiled code.
    PPC_FUNC(rex_sub_82132740) {
        REXLOG_TRACE("Stubbed sub_82132740 called");
        ctx.r3.s64 = 0;
    }

    PPC_FUNC(rex_sub_82132820) {
        REXLOG_TRACE("Stubbed sub_82132820 called");
        ctx.r3.s64 = 0;
    }

    // XAM UI Stubs - Returning 0 (Success/Not implemented)
    PPC_FUNC(rex_sub_82131C20) { ctx.r3.s64 = 0; } // XamShowFriendsUI
    PPC_FUNC(rex_sub_82131C28) { ctx.r3.s64 = 0; } // XamShowGamerCardUIForXUID
    PPC_FUNC(rex_sub_82131C30) { ctx.r3.s64 = 0; } // XamShowAchievementsUI
    PPC_FUNC(rex_sub_82131C38) { ctx.r3.s64 = 0; } // XamShowPlayerReviewUI
    PPC_FUNC(rex_sub_82131C40) { ctx.r3.s64 = 0; } // XamShowMarketplaceUI
}
