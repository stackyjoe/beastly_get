#include <atomic>
#include <chrono>
#include <iostream>
#include <fstream>
#include <thread>

#include <fmt/core.h>

#include "egg_timer.hpp"
#include "getter.hpp"
#include "url_parser.hpp"

void completion_handler(boost::beast::error_code const &ec,
                        [[maybe_unused]] size_t bytes_read,
                        std::string file_destination,
                        beastly_connection &resources) {

    // on read failure don't write to disk.
    if(ec) {
        fmt::print("Download failed with error code: {}\n", ec.message());
        return;
    }

    std::ofstream output_file(file_destination, std::ios::binary);
    auto contents = resources.take_body();

    output_file.write(contents.c_str(), contents.size());
    output_file.close();
    fmt::print("wrote to file\n");
}

int main(int argc, char** argv)
{
    if(argc <2 ) {
        return EXIT_FAILURE;
    }

    std::unique_ptr<std::atomic<size_t>[]> counters(nullptr);
    getter get;

    if(argc == 2) {
        std::string file_location { argv[1] };

        std::ifstream list_file(file_location, std::ios::in);
        std::vector<std::tuple<std::string, std::string>> pairs;

        std::string url;
        std::string save_dest;

        while(not list_file.eof()) {
            list_file >> url;
            list_file >> save_dest;

            pairs.push_back(std::make_tuple(url, save_dest));
        }

        const size_t len = pairs.size();
        counters = std::make_unique<std::atomic<size_t>[]>(len);
        // Destructor of this vector calls destructor of futures which wait on the associated promises
        std::vector<std::future<bool>> futures(pairs.size());

        for(std::size_t download_id = 0; download_id < len; ++download_id) {
            auto &pair = pairs[download_id];
            counters[download_id] = 0;

            auto prog = egg_timer(1000000,
                                [download_id](size_t completed, [[maybe_unused]] size_t total) {
                                    fmt::print("Download {} has downloaded {} since last update.\n", download_id, completed);
                                }
            );

            auto comp = [download_id, file_dest=std::get<1>(pair)](boost::beast::error_code const &ec, [[maybe_unused]] size_t bytes_read, beastly_connection &resources) {

                        completion_handler(ec, bytes_read, file_dest, resources);
                        (void)download_id;
            };


            futures.emplace_back(get.get(std::get<0>(pair),
                                 std::move(prog),
                                 std::move(comp)));
        }

        return EXIT_SUCCESS;
    }

    std::string url { argv[1] };
    std::string file_dest { argv[2] };
    auto prog = egg_timer(1000000,
                        [](size_t completed, [[maybe_unused]] size_t total) {
                            fmt::print("Downloaded {} out of {} since last update.\n", completed, std::max(total,completed));
                        }
    );

    auto cmp = [file_dest](boost::beast::error_code const &ec,
            [[maybe_unused]] size_t bytes_read,
             beastly_connection &resources) {
        completion_handler(ec, bytes_read, file_dest, resources);
    };

    try {

        auto f = get.get(url, std::move(prog), std::move(cmp));

        // Future goes out of scope, std::future's destructor waits on the promise,
        // so the process doesn't complete until the download calls the completion handler.
    }
    catch(std::exception const &e) {
        fmt::print("An exception occurred: {}\n", e.what());
    }


    return EXIT_SUCCESS;
}
