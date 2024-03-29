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

	namespace FileStatuses {
		static constexpr FileStatus C = FileStatus::Created;
		static constexpr FileStatus D = FileStatus::Deleted;
		static constexpr FileStatus M = FileStatus::Modified;
		static constexpr FileStatus CD = FileStatus::Created | FileStatus::Deleted;
		static constexpr FileStatus CM = FileStatus::Created | FileStatus::Modified;
		static constexpr FileStatus DM = FileStatus::Deleted | FileStatus::Modified;
		static constexpr FileStatus All = FileStatus::Created | FileStatus::Deleted | FileStatus::Modified;
	} // namespace FileStatuses

	struct FileInformation {
		FileType type;
		std::string path;
		std::filesystem::file_time_type last_modified;
		FileStatus status = FileStatus::Created;

		auto to_path() const { return std::filesystem::path { path }; }
		auto is_valid() const { return std::filesystem::is_regular_file(to_path()); }
	};

	class FileWatcher {
	public:
		explicit FileWatcher(const std::filesystem::path& path, std::chrono::duration<int, std::milli> in_delay = std::chrono::milliseconds(2000));

		void on_created(const std::function<void(const FileInformation&)>& activation_function);
		void on_modified(const std::function<void(const FileInformation&)>& activation_function);
		void on_deleted(const std::function<void(const FileInformation&)>& activation_function);
		void on_created_or_deleted(const std::function<void(const FileInformation&)>& activation_function);
		void on(FileStatus info, const std::function<void(const FileInformation&)>& activation_function);

		void add_watched_paths(const std::filesystem::path& path);

		~FileWatcher() { stop(); }

	private:
		/// @brief OS specific foreach (Apple-Clang does not support execution::par)
		/// @param info file information
		/// @param activations all functions
		static void for_each(const FileInformation& info, const std::vector<std::function<void(const FileInformation&)>>&);

		void start(const std::function<void(const FileInformation&)>& activation_function);
		void stop();
		void loop_until();

		std::vector<std::function<void(const FileInformation&)>> activations;
		std::filesystem::path root;
		std::chrono::duration<int, std::milli> delay;
		std::unordered_map<std::string, FileInformation, StringHash, std::equal_to<>> paths {};
		std::unordered_set<std::string, StringHash> additional_paths {};
		std::atomic_bool running { true };
		std::thread thread;
	};

} // namespace AssetManager
