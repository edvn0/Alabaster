#pragma once

#include "utilities/StringHash.hpp"

#include <chrono>
#include <filesystem>
#include <functional>
#include <thread>
#include <vector>

namespace AssetManager {

	static constexpr auto bit(auto x) { return 1 << x; }

	enum class FileType : std::uint8_t { DIRECTORY, UNKNOWN, TXT, PNG, TTF, JPEG, JPG, SPV, VERT, FRAG, OBJ, JSON, SCENE };
	enum class FileStatus : std::uint8_t { Created = bit(0), Deleted = bit(1), Modified = bit(2) };

	static constexpr auto operator|(FileStatus left, FileStatus right)
	{
		return static_cast<FileStatus>(static_cast<int>(left) | static_cast<int>(right));
	}

	static constexpr auto operator&(FileStatus left, FileStatus right)
	{
		return static_cast<FileStatus>(static_cast<int>(left) & static_cast<int>(right));
	}

	struct FileInformation {
		FileType type;
		std::string_view path;
		std::filesystem::file_time_type last_modified;
		FileStatus status = FileStatus::Created;

		auto to_path() const { return std::filesystem::path { path }; }
		auto is_valid() const { return std::filesystem::is_regular_file(to_path()); }
	};

	class FileWatcher {
	public:
		explicit FileWatcher(const std::filesystem::path& path, std::chrono::duration<int, std::milli> in_delay = std::chrono::milliseconds(2000));
		void stop();

		void on_created(const std::function<void(const FileInformation&)>& activation_function);
		void on_modified(const std::function<void(const FileInformation&)>& activation_function);
		void on_deleted(const std::function<void(const FileInformation&)>& activation_function);
		void on_created_or_deleted(const std::function<void(const FileInformation&)>& activation_function);

		~FileWatcher() { stop(); }

	private:
		void start(const std::function<void(const FileInformation&)>& activation_function);
		void loop_until();

		std::vector<std::function<void(const FileInformation&)>> activations;
		std::filesystem::path root;
		std::chrono::duration<int, std::milli> delay;
		std::unordered_map<std::string, FileInformation, StringHash, std::equal_to<>> paths {};
		std::atomic_bool running { true };
		std::thread thread;
	};

} // namespace AssetManager
