#pragma comment(lib, "../SDK/lib/bedrock_server_api.lib")
#pragma comment(lib, "../SDK/lib/bedrock_server_var.lib")
#pragma comment(lib, "../SDK/lib/SymDBHelper.lib")
#pragma comment(lib, "../SDK/lib/LiteLoader.lib")

#include <llapi/mc/Player.hpp>
#include <llapi/mc/Container.hpp>
#include <llapi/mc/ItemStack.hpp>

void reloadItem(Player& player, int id, int auxValue) {
    if (!player.getSelectedItem().isNull()) {
        return;
    }
    int slot = player.getSelectedItemSlot();
    Container& inventory = player.getInventory();
    for (int i = 0; i < inventory.getContainerSize(); ++i) {
        if (i == slot) {
            continue;
        }
        ItemStack const& itemStack = inventory.getItem(i);
        if (!itemStack.sameItem(id, auxValue)) {
            continue;
        }
        if (i < 9) {
            player.setSelectedSlot(i);
            return;
        }
        ItemStack newItemStack = itemStack.clone();
        inventory.setItem(i, ItemStack::EMPTY_ITEM);
        player.setSelectedItem(newItemStack);
        player.sendInventory(true);
        return;
    }
}

#include <llapi/mc/ItemStackBase.hpp>
#include <llapi/mc/Actor.hpp>
#include <llapi/mc/ActorUniqueID.hpp>
#include <llapi/mc/Level.hpp>
#include <llapi/HookAPI.h>
#include <llapi/ScheduleAPI.h>
#include <llapi/GlobalServiceAPI.h>

TInstanceHook(void, "?useItem@Player@@UEAAXAEAVItemStackBase@@W4ItemUseMethod@@_N@Z", Player, ItemStackBase& a2, ItemUseMethod a3, bool a4) {
    int id = a2.getId();
    int auxValue = a2.getAuxValue();
    original(this, a2, a3, a4);
    if (!a2.isNull()) {
        return;
    }
    ActorUniqueID const& uniqueID = getUniqueID();
    Schedule::nextTick([uniqueID, id, auxValue]() {
        Player* player = Global<Level>->getPlayer(uniqueID);
        if (!player) {
            return;
}
        reloadItem(*player, id, auxValue);
    });
}
TInstanceHook(bool, "?hurtAndBreak@ItemStackBase@@QEAA_NHPEAVActor@@@Z", ItemStackBase, int a2, Actor& a3) {
    int id = getId();
    int auxValue = getAuxValue();
    bool result = original(this, a2, a3);
    if (!isNull() || !a3.isPlayer()) {
        return result;
    }
    ActorUniqueID const& uniqueID = a3.getUniqueID();
    Schedule::nextTick([uniqueID, id, auxValue]() {
        Player* player = Global<Level>->getPlayer(uniqueID);
        if (!player) {
            return;
        }
        reloadItem(*player, id, auxValue);
    });
    return result;
}
