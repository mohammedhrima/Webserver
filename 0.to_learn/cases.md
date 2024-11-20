+ b9a mhangi, sseyfet 408 ta3 time out o b9a mhangi
    + "POST / HTTP/1.1\r\nContent-Length: 20\r\nHost: localhost:17000\r\n\r\naa\r\n" 

+ "POST / HTTP/1.1\r\nContent-Length: 2\r\nHost: localhost:17000\r\n\r\naa\r\n" 

+ kaytcrea l file mais mafih walo, khass tkon fih "as"
    + "POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\nHost: localhost:17000\r\n\r\n2\r\nas\r\n0\r\n\r\n" 

+ size ila 3tito lik fih characheter mishi f l HEXA f chunked makatshikihsh expl: 2x, erf

+ hadi katele not found:
    + "GET /kl/src HTTP/1.1\r\nHost: localhost:17000\r\n\r\n" | nc 127.0.0.1 17000/
    rah zedt hadi f config file :
    location /kl {
            methods     : GET POST DELETE;
            index       : file.html;
            destination : ./dest;
        }
    So khassha tdkhel l src 7it kl khass t3ewed b root lil fo9 which is ./serv o ghatweli ./serv/src

+ POST / HTTP/1.1\r\nContent-Length: 20\r\nHost: localhost:17000\r\n\r\naa\r\n"
    + kayhangi walakin had lmra dert ctrl + c deghya wmn wraha runit hada 
        + GET /kl/ HTTP/1.1\r\nHost: localhost:17000\r\n\r\n" 
        + w hywa ywgef server, mishi segfault walakin 7bess which isnt good, khasso may7besessh
        + hada hwa line lekher likteb 9bel may7bess :
           + timeout: Error: line 442 server not found

+ 3andekl l indexing kaytkhewer ila kan l file fih emoji wla space f ssmiya
+ hwa khdam ila ktbt ssmiya direct flpath, but makatdirhash mzyana f indexing, (3andek shi tkwira f l html code likatgeneri)


