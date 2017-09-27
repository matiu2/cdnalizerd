%%{
    machine url;

    action scheme_end {
      scheme = std::string(start, p);
    }
    action host_start {
      start = p;
    }
    action host_end {
      host = std::string(start, p);
    }
    action port_start {
        start = p;
    }
    action port_end {
        port = std::string(start, p);
    }
    action user_start {
        start = p;
    }
    action user_end {
        user = std::string(start, p);
    }
    action password_start {
        start = p;
    }
    action password_end {
        password = std::string(start, p);
    }
    action path_start {
        start = p;
    }
    action path_end {
        path = std::string(start, p);
    }
    action search_start {
        start = p;
    }
    action search_end {
        search = std::string(start, p);
    }

}%%
