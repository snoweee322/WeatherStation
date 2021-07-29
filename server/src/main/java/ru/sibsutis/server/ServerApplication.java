package ru.sibsutis.server;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.core.io.InputStreamResource;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import ru.sibsutis.server.entities.WeatherEntity;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.List;


@SpringBootApplication
@Controller
public class ServerApplication {

	@Autowired
	private ServerRepository serverRepository;

	public static void main(String[] args) {
		SpringApplication.run(ServerApplication.class, args);
	}

	@RequestMapping("/")
	public String display(Model model) {
		List<WeatherEntity> list = serverRepository.findFirst10ByOrderByIdDesc();
		model.addAttribute("weathers", list);
		return "Display";
	}

	@RequestMapping("/last")
	public String last(Model model) {
		List<WeatherEntity> list = serverRepository.findFirst1000ByOrderByIdDesc();
		model.addAttribute("list", list);
		return "Last";
	}

	@RequestMapping("/history")
	public String history() {
		return "History";
	}

	@RequestMapping(path = "/download", method = RequestMethod.GET)
	public ResponseEntity<Resource> download() throws IOException {
		ObjectMapper mapper = new ObjectMapper();
		mapper.writeValue(new File("src/main/resources/temp/data.json"), serverRepository.findAll());
		File file = new File("src/main/resources/temp/data.json");
		InputStreamResource resource = new InputStreamResource(new FileInputStream(file));
		HttpHeaders headers = new HttpHeaders();
		headers.add(HttpHeaders.CONTENT_DISPOSITION, "attachment; filename = data.json");
		return ResponseEntity.ok()
				.headers(headers)
				.contentLength(file.length())
				.contentType(MediaType.APPLICATION_OCTET_STREAM)
				.body(resource);
	}
}
