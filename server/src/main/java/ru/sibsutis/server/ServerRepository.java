package ru.sibsutis.server;

import org.springframework.data.repository.CrudRepository;
import ru.sibsutis.server.entities.WeatherEntity;

import java.util.List;

public interface ServerRepository extends CrudRepository<WeatherEntity, Integer> {
    List<WeatherEntity> findFirst10ByOrderByIdDesc();
    List<WeatherEntity> findFirst1000ByOrderByIdDesc();
    List<WeatherEntity> findAllByOrderByIdAsc();
}
