#include "pch.h"

#include <filesystem>
#include <execution>

#define NOMINMAX
#include <Windows.h>
#include <gdiplus.h>

#include <luawrap.hpp>
#include <cube.hpp>

#include "stopwatch.hpp"

#include "aulut_macro.h"
#include "aulut_cache.hpp"

namespace aulut {
	const lut_t& get_lutdata(Lua::StackIndex idx) {
		using Lua::Type;
		switch (idx.type()) {
		case Type::String: {
			std::filesystem::path path{ idx.toStringTrusted() };
			auto& cache = get_cache();
			if (auto found = cache.find(path); found == cache.end()) {
				try {
					auto [emplaced, _] = cache.try_emplace(path, my_cube_from_file(path));

					return emplaced->second;
				}
				catch (CubeHpp::cube_exception& e) {
					throw Lua::Exception{ "CubeHpp::cube_exception: {}", e.what() };
				}
			}
			else {
				return found->second;
			}
		}
		case Type::Userdata:
			return idx.checkClass<itr_t>().get()->second;
		default:
			idx.getState().throwTypeException(1, "string or aulut_data");
		}
	}
	
	int l_main(lua_State* l) {
		Lua::State L{l};
		try {
			stopwatch s{};
			const auto& lut = get_lutdata(L[1]);

			float af;
			if (L[2].isNumber()) {
				af = static_cast<float>(L[2].toNumber());
				L.remove(2);
			}
			else {
				af = 1.f;
			}
			auto data = L[2].toUserdata<CubeHpp::BGRA32>();
			if (data == nullptr) {
				throw Lua::ArgException{ 1, "Invalid image data" };
			}
			int w = L[3].toInteger();
			int h = L[4].toInteger();

			try {
				//stopwatch s{};
				lut.apply(data, w * h, af);
			}
			catch (CubeHpp::cube_exception& e) {
				throw Lua::Exception{ "LUT apply error ({})", e.what() };
			}

			L.setTop(2);

			return 1;
		}
		catch (Lua::Exception& e) {
			L.push(e);
		}
		L.error();
	}

	void generate_std1d(CubeHpp::BGRA32* ptr) {
		for (int z = 0; z < 3; z++) {
			for (int y = 0; y < 70; y++) {
				for (int x = 0; x < 256; x++) {
					ptr[x + (y + z * 80) * 256] = {
						static_cast<uint8_t>(x),
						static_cast<uint8_t>(x),
						static_cast<uint8_t>(x),
						255
					};
				}
			}
			for (int y = 70; y < 80; y++) {
				for (int x = 0; x < 256; x++) {
					ptr[x + (y + z * 80) * 256] = { 0,0,0,255 };
				}
			}
		}
	}

	void generate_std3d(CubeHpp::BGRA32* ptr) {
		for (int zy = 0; zy < 8; zy++) {
			for (int zx = 0; zx < 8; zx++) {
				for (int y = 0; y < 64; y++) {
					for (int x = 0; x < 64; x++) {
						ptr[(zy * 64 + y) * 512 + zx * 64 + x] = {
							static_cast<uint8_t>(((zx + zy * 8) * 255 + 31) / 63),
							static_cast<uint8_t>((y * 255 + 31) / 63),
							static_cast<uint8_t>((x * 255 + 31) / 63),
							255
						};
					}
				}
			}
		}
	}
	
	// std(integer type, userdata data)
	int l_std(lua_State* _l) {
		Lua::State L{_l};
		try {
			auto type = L.checkInteger(1);
			auto data = L.toUserdata<CubeHpp::BGRA32>(2);
			if (data == nullptr) return 0;

			switch (type) {
				case 1:
					generate_std1d(data);
					break;
				case 3:
					generate_std3d(data);
					break;
				default:
					throw Lua::ArgException(1, "invalid typenum");
			}
			L.setTop(2);
			return 1;
		}
		catch (Lua::Exception& e) {
			L.push(e);
		}
		L.error();
	}
	
	int l_save(lua_State* _l) {
		Lua::State L{_l};
		try {
			auto path_str_opt = L[1].toStringStrict();

			auto data = L[2].toUserdata<CubeHpp::BGRA32>();
			if (data == nullptr) return 0;

			auto type = L[3].checkInteger();
			if (!(type == 1 || type == 3)) throw Lua::ArgException{ 1, "invalid cube type" };

			auto title = L[4].toStringStrict().transform([](auto x) { return std::string{x}; });

			auto path{ path_str_opt
				.transform([](auto x) {
					return std::filesystem::path{x};
				}).or_else([]() -> std::optional<std::filesystem::path> {
					std::wstring buf;
					buf.resize(MAX_PATH);
					::OPENFILENAMEW ofn{
						.lStructSize{sizeof ofn},
						.lpstrFilter = L".cube file(*.cube)\0*.cube\0",
						.lpstrCustomFilter = nullptr,
						.nFilterIndex = 0,
						.lpstrFile = buf.data(),
						.nMaxFile = MAX_PATH + 1,
						.nMaxFileTitle = 1,
						.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_ENABLESIZING,
						.lpstrDefExt = L"cube",
					};
					if (::GetSaveFileNameW(&ofn) == FALSE) throw Lua::ArgException{ 1, "ファイルパスは指定されませんでした" };
					return std::filesystem::path{ std::move(buf) };
				}).value() };

			try {
				CubeHpp::cube_bgra32 cube{ static_cast<CubeHpp::CubeType>(type), std::move(title), data };

				if (L[5].isNoneOrNil()) {
					cube.save(path);
				}
				else {
					if (!L[5].isTable()) {
						L.throwTypeException<Lua::Type::Table>(5);
					}
					auto mn = static_cast<float>(L.push(1), L.getTable(5), L.toNumber(-1));
					auto mx = static_cast<float>(L.push(2), L.getTable(5), L.toNumber(-1));
					cube.save(path, mn, mx);
				}
			}
			catch (CubeHpp::cube_exception& e) {
				throw Lua::Exception{ "cube construct error : {}", typeid(e).name() };
			}

			return 0;
		}
		catch (Lua::Exception& e) {
			L.push(e);
		}
		L.error();
	}
	
	int l_image(lua_State* _l) {
		Lua::State L{_l};
		try {
			auto& lut = get_lutdata(L[1]);
			auto data = L[2].toUserdata<CubeHpp::BGRA32>();

			using CubeHpp::CubeType;
			
			switch (lut.type()) {
				case CubeType::c_1d:
					generate_std1d(data);
					try {
						lut.apply(data, 256 * 240, 1.f);
					}
					catch (CubeHpp::cube_exception& e) {
						throw Lua::Exception{ e.what() };
					}
					break;
				case CubeType::c_3d:
					generate_std3d(data);
					try {
						lut.apply(data, 512 * 512, 1.f);
					}
					catch (CubeHpp::cube_exception& e) {
						throw Lua::Exception{ e.what() };
					}
					break;
				default:
					throw Lua::Exception{ "Invalid LUT data" };
			}

			L.setTop(2);
			return 1;
		}
		catch (Lua::Exception& e) {
			L.push(e);
		}
		L.error();
	}
	
	int l_preload(lua_State* _l) {
		Lua::State L{_l};
		try {
			auto path = std::filesystem::path{ L[1].checkStringStrict()};

			auto& cache = get_cache();
			auto itr = cache.find(path);

			bool exist;
			if (itr != cache.end()) {
				exist = true;
			}
			else {
				try {
					std::tie(itr, std::ignore) = cache.try_emplace(path, my_cube_from_file(path));
				}
				catch (CubeHpp::cube_exception& e) {
					throw Lua::Exception{ "cube construct error: {}", typeid(e).name() };
				}
				exist = false;
			}

			(void)L.newClass<itr_t>(itr);
			L.push(exist);

			return 2;
		}
		catch (Lua::Exception& e) {
			L.push(e);
		}
		L.error();
	}
	
	int l_exist(lua_State* _l) {
		Lua::State L{_l};
		try {
			auto path = std::filesystem::path{ L.checkString(1) };
			const auto& cache = get_cache();
			auto found = cache.find(path);

			L.push(found != cache.end());

			return 1;
		}
		catch (Lua::Exception& e) {
			L.push(e);
		}
		L.error();
	}
	
	int l_reset(lua_State* _l) {
		Lua::State L{_l};
		try {
			auto& cache = get_cache();

			using Lua::Type;
			switch (L[1].type()) {
				case Type::None:
					cache.clear();
					return 0;
				case Type::String: {
					std::filesystem::path path{ L[1].toStringTrusted() };
					L.pushBoolean(cache.erase(path));
					return 1;
				}
				default:
					L.throwTypeException(1, L.typeName(Lua::Type::String));
			}
		}
		catch (Lua::Exception& e) {
			L.push(e);
		}
		L.error();
	}

	int l_cached_list(lua_State* _l) {
		Lua::State L{_l};
		try {
			auto& cache = get_cache();

			(void)L.newClass<itr_t>(cache.cbegin());
			L.pushCClosure(1, [](lua_State* _l) {
				using namespace Lua::literals;
				Lua::State L{ _l };
				try {
					auto& cache = get_cache();

					auto& itr = *L[1_upvalue].toClass<itr_t>();

					if (itr == cache.cend()) return 0;

					L.pushString(itr->first.string());
					(void)L.newClass<itr_t>(itr);

					itr++;

					return 2;
				}
				catch (Lua::Exception& e) {
					L.push(e);
				}
				L.error();
			});
			return 1;
		}
		catch (Lua::Exception& e) {
			L.push(e);
		}
		L.error();
	}

	constinit auto reg = std::to_array<Lua::Reg>({
		{"main", l_main},
		{"std", l_std},
		{"save", l_save},
		{"image", l_image},
		{"preload", l_preload},
		{"exist", l_exist},
		{"reset", l_reset},
		{"cached_list", l_cached_list},
	});
}

int __cdecl luaopen_aulut(lua_State* _l) {
	Lua::State L{_l};
	try {
		#ifdef AULUT_INTRINSICS_MODE
			#if AULUT_INTRINSICS_MODE == AULUT_INTRINSICS_MODE_AVX512 && !defined(SIMDEN_EMULATE_INTRINSICS)
				if (!CubeHpp::AVX512::is_supported(simden::get_intrinsics_flag())) {
					throw Lua::Exception{ "Your CPU is not supported this feature." };
				}
			#elif AULUT_INTRINSICS_MODE == AULUT_INTRINSICS_MODE_AVX2 && !defined(SIMDEN_EMULATE_INTRINSICS)
				if (!CubeHpp::AVX2::is_supported(simden::get_intrinsics_flag())) {
					throw Lua::Exception{ "Your CPU doesn't support this feature." };
				}
			#elif AULUT_INTRINSICS_MODE == AULUT_INTRINSICS_MODE_SSE4 && !defined(SIMDEN_EMULATE_INTRINSICS)
				if (!simden::intrinsics<simden::intrinsics_flag(simden::itSSE41)> ::is_supported(simden::get_intrinsics_flag())) {
					throw Lua::Exception{ "Your CPU doesn't support this feature." };
				}
			#endif
		#endif

		L.registerLib("aulut", aulut::reg);

		L.newRegClassMetatable<aulut::itr_t>(); L.pop(1);

		return 1;
	}
	catch (Lua::Exception& e) {
		L.push(e);
	}
	L.error();
}
