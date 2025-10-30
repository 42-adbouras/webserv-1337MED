#pragma once

#define G_TIME_OUT 10000 // globale time-out for poll() events in ms
#define CLIENT_HEADER_TIMEOUT 60 // second
#define CLIENT_BODY_TIMEOUT 60 // second
#define KEEPALIVE_TIMEOUT 75 // second

// void    closeSockets(std::vector<std::pair<int, struct sockaddr*> >& pollfd) {
//     for (size_t i = 0; i < pollfd.size(); i++)
//     {
//         close(pollfd[i].first);
//     }
// }