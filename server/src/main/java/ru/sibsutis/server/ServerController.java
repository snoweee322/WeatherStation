package ru.sibsutis.server;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;
import ru.sibsutis.server.entities.WeatherEntity;

import java.sql.Timestamp;
import java.util.List;

@RestController
@RequestMapping(path = "/api")
public class ServerController {
    @Autowired
    private ServerRepository serverRepository;

    @PostMapping(path = "/add")
    public @ResponseBody String addNewWeather (@RequestParam long time, @RequestParam Float temp, @RequestParam Float hum) {
        Timestamp timestamp = new Timestamp(time * 1000); // java needs ms
        WeatherEntity weatherEntity = new WeatherEntity();
        weatherEntity.setTimestamp(timestamp);
        weatherEntity.setTemp(temp);
        weatherEntity.setHum(hum);
        serverRepository.save(weatherEntity);
        return "Done!";
    }

    @GetMapping(path = "/getAll")
    public @ResponseBody Iterable<WeatherEntity> getAll() {
        return serverRepository.findAllByOrderByIdAsc();
    }

    @GetMapping("/get")
    public @ResponseBody List<WeatherEntity> get10() {
        return serverRepository.findFirst10ByOrderByIdDesc();
    }
}
