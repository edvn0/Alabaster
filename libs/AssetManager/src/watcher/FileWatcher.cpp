#include "watcher/FileWatcher.hpp"

#include "core/Common.hpp"
#include "core/exceptions/AlabasterException.hpp"

#include <algorithm>
#include <execution>

namespace AssetManager {

	constexpr auto to_filetype(const auto& extension) -> FileType
	{
		using namespace Alabaster;
		if (extension == ".txt")
			return FileType::TXT;
		else if (extension == ".png")
			return FileType::PNG;
		else if (extension == ".ttf")
			return FileType::TTF;
		else if (extension == ".jpeg")
			return FileType::JPEG;
		else if (extension == ".jpg")
			return FileType::JPG;
		else if (extension == ".spv")
			return FileType::SPV;
		else if (extension == ".vert")
			return FileType::VERT;
		else if (extension == ".frag")
			return FileType::FRAG;
		else if (extension == ".obj")
			return FileType::OBJ;
		else if (extension == ".json")
			return FileType::JSON;
		else if (extension == ".scene")
			return FileType::SCENE;
		else
			return FileType::UNKNOWN;
	}

	FileWatcher::FileWatcher(const std::filesystem::path& in_path, std::chrono::duration<int, std::milli> delay)
		: root(in_path)
		, delay(delay)
	{
		Alabaster::assert_that(std::filesystem::is_directory(root), "File watcher API currently only supports directories.");
		for (const auto& file : std::filesystem::recursive_directory_iterator { root }) {
			const auto path = file.path().string();
			const auto type_if_not_directory = std::filesystem::is_directory(file) ? FileType::DIRECTORY : to_filetype(file.path().extension());

			paths[path] = FileInformation {
				.type = type_if_not_directory,
				.path = path,
				.last_modified = std::filesystem::last_write_time(path),
			};
		}

		thread = std::thread(&FileWatcher::loop_until, this);
	}

	void FileWatcher::loop_until()
	{
		while (running) {
			std::this_thread::sleep_for(delay);

			auto path_iterator = paths.begin();
			while (path_iterator != paths.end()) {
				if (!std::filesystem::exists(path_iterator->first)) {
					path_iterator->second.status = FileStatus::Deleted;
					std::for_each(std::execution::par, activations.begin(), activations.end(),
						[&file_info = path_iterator->second](const auto& func) { func(file_info); });
					path_iterator = paths.erase(path_iterator);
				} else {
					path_iterator++;
				}
			}

			for (auto& file : std::filesystem::recursive_directory_iterator(root)) {
				auto current_file_last_write_time = std::filesystem::last_write_time(file);
				const auto path = file.path();
				const auto view = file.path().string();

				// File creation
				if (!paths.contains(view)) {
					const auto type_if_not_directory
						= std::filesystem::is_directory(file) ? FileType::DIRECTORY : to_filetype(file.path().extension());

					paths[file.path().string()] = FileInformation {
						.type = type_if_not_directory, .path = view, .last_modified = current_file_last_write_time, .status = FileStatus::Created
					};

					const auto current = paths[file.path().string()];
					std::for_each(std::execution::par, activations.begin(), activations.end(), [&current](const auto& func) { func(current); });
					// File modification
				} else {
					auto current = paths[file.path().string()];

					if (current.last_modified != current_file_last_write_time) {
						current.last_modified = current_file_last_write_time;

						std::for_each(std::execution::par, activations.begin(), activations.end(), [&current](const auto& func) { func(current); });
					}
				}
			}
		}
	}

	void FileWatcher::start(const std::function<void(const FileInformation&)>& in) { activations.push_back(in); }

	static constexpr auto register_callback(FileStatus status, auto& activations, auto&& function)
	{
		auto func = [activation = function, status = status](const auto& file) {
			const auto is_given_status = static_cast<bool>(file.status & status);
			if (is_given_status)
				activation(file);
		};
		activations.push_back(func);
	}

	void FileWatcher::on_created(const std::function<void(const FileInformation&)>& in) { register_callback(FileStatus::Created, activations, in); }

	void FileWatcher::on_modified(const std::function<void(const FileInformation&)>& in) { register_callback(FileStatus::Modified, activations, in); }

	void FileWatcher::on_deleted(const std::function<void(const FileInformation&)>& in) { register_callback(FileStatus::Deleted, activations, in); }

	void FileWatcher::on_created_or_deleted(const std::function<void(const FileInformation&)>& in)
	{
		register_callback(FileStatus::Deleted | FileStatus::Created, activations, in);
	}

	void FileWatcher::stop()
	{
		try {
			running = false;
			thread.join();
		} catch (const std::exception& e) {
			Alabaster::Log::info("Already joined this thread. Message: {}", e.what());
		}
	}

} // namespace AssetManager
