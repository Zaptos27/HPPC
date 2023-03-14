void simulate(const Sim_Configuration config) {
    Water water_world = Water();

    grid_t water_history[10];
    auto begin = std::chrono::steady_clock::now();
    #pragma acc data copyin(water_world) create(water_history[0:10])
    {
    int i = 0;
    for (uint64_t t = 0; t < config.iter; ++t) {
        integrate(water_world, config.dt,  config.dx, config.dy, config.g);
        if (t % config.data_period == 0) {
            #pragma acc update host(water_world)
            water_history[i] = water_world.e;
            #pragma acc update device(water_history[i])
            i++;
        }
    }
    }
    auto end = std::chrono::steady_clock::now();
    std::vector <grid_t> water_history2;
    
    for(int i = 0; i<10;i++){
        water_history2.push_back(water_history[i]);
        std::cout << "test : " << std::accumulate(water_history2[i].front().begin(), water_history2[i].back().end(), 0.0) << "\n";
    }
    to_file(water_history2, config.filename);

    std::cout << "checksum: " << std::accumulate(water_world.e.front().begin(), water_world.e.back().end(), 0.0) << std::endl;
    std::cout << "elapsed time: " << (end - begin).count() / 1000000000.0 << " sec" << std::endl;
}