#include "memory.h"
#include <thread>
#include <array>

namespace offset {
	//CLIENT
	constexpr ::std::ptrdiff_t dwLocalPlayer = 0xDE997C;
	constexpr ::std::ptrdiff_t dwEntityList = 0x4DFEECC;
	constexpr ::std::ptrdiff_t dwClientState = 0x59F19C;
	//PLAYER
	constexpr ::std::ptrdiff_t m_hMyWeapons = 0x2E08;
	//BASE ATTRIBUTABLE
	constexpr ::std::ptrdiff_t m_flFallbackWear = 0x31E0;
	constexpr ::std::ptrdiff_t m_nFallbackPaintKit = 0x31D8;
	constexpr ::std::ptrdiff_t m_nFallbackSeed = 0x31DC;
	constexpr ::std::ptrdiff_t m_nFallbackStatTrak = 0x31E4;
	constexpr ::std::ptrdiff_t m_iItemDefinitionIndex = 0x2FBA;
	constexpr ::std::ptrdiff_t m_iItemIDHigh = 0x2FD0;
	constexpr ::std::ptrdiff_t m_iEntityQuality = 0x2FBC;
	constexpr ::std::ptrdiff_t m_iAccountID = 0x2FD8;
	constexpr ::std::ptrdiff_t m_OriginalOwnerXuidLow = 0x31D0;
}
//set skins to apply here
constexpr const int GetWeaponPaint(const short& itemDefinition)
{
	switch (itemDefinition)
	{
	case 1: return 37; // deagle
	case 4: return 694; // glock
	case 7: return 180; // ak
	case 9: return 344;// awp
	case 60: return 326; // m4a1s
	case 61: return 504; // usp
	case 515: return 568; // knife
	default: return 0;
	}
}


int main()
{
	const auto memory = Memory{ "csgo.exe" };
	//get our module address
	const auto client = memory.GetModuleAddress("client.dll");
	const auto engine = memory.GetModuleAddress("engine.dll");
	//hack loop
	while (true) 
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
		const auto& localPlayer = memory.Read<std::uintptr_t>(client + offset::dwLocalPlayer);
		const auto& weapons = memory.Read<std::array<unsigned long, 8>>(localPlayer + offset::m_hMyWeapons);

		//LOCAL PLAYER WEAPON ITERATION
		for (const auto& handle : weapons) 
		{
			const auto& weapon = memory.Read<std::uintptr_t>((client + offset::dwEntityList + (handle & 0xFFF) * 0x10) - 0x10);

			//make sure weapon is valid
			if (!weapon)
				continue;

			//see if we want to apply skin
			if (const auto paint = GetWeaponPaint(memory.Read<short>(weapon + offset::m_iItemDefinitionIndex))) 
			{
				const bool shouldUpdate = memory.Read<std::int32_t>(weapon + offset::m_nFallbackPaintKit) != paint;

				//force weapon to use fallback values
				memory.Write<std::int32_t>(weapon + offset::m_iItemIDHigh, -1);
				memory.Write<std::int32_t>(weapon + offset::m_nFallbackPaintKit, paint);
				memory.Write<float>(weapon + offset::m_flFallbackWear, 0.1f);

				if (shouldUpdate)
					memory.Write<std::int32_t>(memory.Read<std::uintptr_t>(engine + offset::dwClientState) + 0x174, -1);
			}
		}
	}

	return 0;
}
