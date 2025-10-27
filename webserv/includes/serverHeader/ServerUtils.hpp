#pragma once

void    closeSockets(std::vector<std::pair<int, struct sockaddr*> >& pollfd) {
    for (size_t i = 0; i < pollfd.size(); i++)
    {
        close(pollfd[i].first);
    }
}