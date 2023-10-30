Тут вкратце описаны все файлы (созданные мной) по содержимому

// // // dog.h // // //

Содержит классы Zoo, Dog и его tag_invoke, вспомогательную структуру Location и JsonFieldsName и DogDirection для магических строк

Zoo - создаёт собаку с уникальным id. 

Dog - собака с реализацией смены направления, скорости и локации, хранит id, имя, направление, скорость, текущую точку и стандартную скорость.

    Также содержит класс Speed с перегруженным оператором умножения для создания точки при умножении на время.
    Класс сделан не копируемым, поскольку не должно быть 2 одинаковых собак в игре

Location - структура местонахождения, находится в namespace model


Почему так? Я решил, что Zoo это неотъемлемая часть игры, а значит она должна его содержать. Поскольку зоопарк возвращает не просто id, а shared_ptr на новую собаку (так логичнее, мы забрали щенка из зоопарка), то и определение собаки должно быть рядом. А оно в свою очередь утаскивает за собой класс Location. В итоге получилось разбитие пространства имён game_unit (часть в dog.h, часть в player.h), это можно решить, отдав Zoo в управление например классу GameDataBaseHolder в файле info_holders.h (о нём ниже), но я настаиваю что зоопарк это часть игры, а не управления над ней, поэтому так.

// // // model.h // // //

Содержит классы Map, Road, Building, Office, Game, GameSession и их tag_invoke, а также класс model::JsonFieldsName для магических строк 

Все классы сохранили свои сигнатуры и поддерживают обратную совместимость со своими прошлыми реализациями (расширение, но неизменность)

Вспомогательный класс Point 

    Перегружен оператор < для сравнения точек, чтобы находить у дороги, какая из точек на самом деле start, а какая end

Класс Road 

    Для задачи передвижения собаки были добавлены параметры:
        Толщина в формате double
        Верхняя правая и левая нижняя точки в формате double, которые выставляются при создании дороги

Класс Map

    Для задачи передвижения собаки были добавлены параметры: (Алгоритм и то как сортируются дороги находятся в описании класса GameSession)

        Приватные вспомогательные классы-компараторы Comparator_horizontal и Comparator_vertical. Почему приватные? Потому что только карта имеет право сравнивать дороги. 

        set horizontal_roads_ и vertical_roads_ для поиска дороги, которой принадлежит заданное местоположение. 
        К сожалению, во имя обратной совместимости, и поскольку тесты написаны так, что принимают список дорог только в том порядке, в котором дали, пришлось сохранить vector<Road>. Получается Map хранит по 2 копии каждой дороги. Печально, но передвижение собаки становится в разы быстрее. Почему не сделать хранение указателей? Так как основные объекты хранятся в векторе. При вставке, указатели на объекты могут инвалидироваться, а никто не гарантирует, что список дорог не может модифицироваться. Можно сделать наоборот, в set основные, и вектор итераторов, но тогда страдает обратная совместимость.
        Они обновляются по мере заполнения vector<Road>
        
        Метод FindRoads. Поиск вертикальной и горизонтальной дороги, которым принадлежит точка
        Почему такой страшный тип вывода? Нам необходимо вернуть дорогу, которой принадлежит точка или же что-то, что показывает, что точка находится вне дороги. Поскольку обновление реализует GameSession, то ничего кроме optional то самое default value нам не даст. Поскольку у нас вертикальные и горизонтальные дороги, то мы используем pair.

        Поле standard_speed. Стандартная скорость на карте.
        Наличие этого поля проверяет tag_invoke. В случае если его нет, то оно остаётся -100, и его исправит Game на свою скорость при вставке.

Класс GameSession

    Метод UpdateOnTime делает update всех собак по координатам. Для этого он пробегает по списку своих собак и для каждой реализует следующий алгоритм:
    Вызывает FindRoads для нынешнего местоположения, вычисляет предполагаемое новое и вызывает FindRoads для него. В случае если одна из дорог осталась прежней, то мы считаем передвижение валидным и присваиваем собаке вычисленное местоположение. Иначе в зависимости от её направления и тому на вертикальной или горизонтальной дороге она находилась, высчитываем границу в которую надо упереться и ставим скорость 0.

        Почему это работает? Используется предположение, что нам дают логичный список дорог (оказалось что это не так, но тесты не падают, а значит можно смело идти к тому, кто писал файл конфига и спрашивать как он себе представляет то, что дорога (0, 10) -> (20, 10) разделена поровну на 2: (0, 10) -> (10, 10) и (10, 10) -> (20, 10)). Если у двух дорог верхний правй угол одинаков, то это или разные по ориентации (ха-ха) дороги, или же одна является частью другой и это уже явно не наша вина. 
        
        В set мы сравниваем верхние границы дороги и считаем, что эта точка уникальна (однозначно определяет положение дороги в set), так как мы разделили дороги на вертикальные и горизонтальные (поскольку мы не умеем двигаться по диагонали, то мы считаем что они существуют отдельно друг от друга). При помощи этого получаем set, сортированный по возрастанию по x или y.

        Затем идёт проверка на принадлежность точки, должны мы сместиться на уровень выше (точка больше чем верхний угол) или ниже (меньше чем нижний) в поиске дороги. Таким образом мы однозначно определяем дорогу (или её отсутствие). Сделано это всё так, что мы просто вызываем метод find от Location. Всё реализовано в компараторах. Если наше перемещение валидно, то мы передвигались вдоль одной дороги, что и подтвердит последующее сравнение. Если нет, то мы перепрыгнули с одну на другую (или в окно) и должны упереться в границу.

    Содержит также shared_ptr от Zoo, который получает от Game при создании

Класс Game

    Игра содержит unordered_map от карты к игровой сессии. Почему не к unordered_set или deque сессий? Чтобы было чему падать в будущих спринтах) Пока что сессия одна на карту, поэтому я оставлю удовольствие подсчёта игроков в сессии и принятия решения создания новой (и удаления старой) на потом.

    Также у игры есть метод GetGameSession, у которого есть параметр created, чтобы в случае отсутствия сессии, она создавалась. Сделано для того, чтобы когда потребуется просто проверка, можно было бы поставить false и не делать новый метод.

    Добавлен параметр стандартной скорости, наличие которого проверяет tag_invoke.

    Добавлена проверка стандартной скорости при добавлении карты. В случае если она меньше 0, подставляется своя.

    Добавлено поле Zoo

// // // player.h // // //

Содержит классы Tokenizer, Player и PlayerLibrary

Класс Tokenizer

    Создаёт токен, необходимый для авторизации

Класс Player

    Класс игрока, содержит собаку, токен и игровую сессию, к которой принадлежит. Запрещён к копированию

    В качестве id использует id собаки (почему нет?). Также с именем

Класс PlayerLibrary

    В уроках это класс Players. Содержит unordered_map от токена к игроку, а также от сессии к игрокам в ней.

    Создаёт игрока и сразу добавляет его в свои списки, которые хранят shared_ptr (вообще всё что можно в shared_ptr), что исключает копирование


// // // http_request_constant.h // // //

Тут хранятся все магические строки, типы ошибок, ответов, запросов и прочее и прочее.

Возможные типы направлений в unordered_map, ответы написаны и в json и в string формате, всё равно json создавать

// // // info_holders.h // // //

Содержит RequestInfo, RequestException, GameDataBaseHolderб Application для передачи в таймер и всё для обработки API запросов

Класс RequestInfo содержит всю необходимую мне информацию для обработки запроса, чтобы не перекидывать весь request. Запрещён к копированию. Поля target и method_allowed обновляются в процессе обработки.

Класс RequestException используется для проброса легальных ошибок (BadRequest и прочее). Типы ошибок реализованы как enum (перечислены в http_request_constant.h ), что упрощает работу с ними (например при обработке используется switch)

Класс GameDataBaseHolder хранит в себе игру (только он и больше никто), библиотеку игроков, токенайзер (то, что делает токены), bool параметры для включения/выключения активного обновления игры и рандомизации стартовой локации

    Также реализована unordered_map, чтобы не делать вечный start_with для target. (Метод выполнения выбирается в HandleURI)
    Все проверки производятся в методах или функциях, которые в случае их провала выкидывают RequestException с соответствующим типом ошибки

    Метод UpdateOnTick создан для передачи в таймер и обновления игры по команде из него

Класс Application создан для того, чтобы передавать метод UpdateOnTick в таймер

// // // logger.h // // //

 Класс для логирования. Работает, сделан по уроку

 // // // request_handler.h // // //

Содержит классы RequestHandler, Timer, работает через mutex

  Тут хранится таймер, потому что для него нужно очень много библиотек, поэтому он здесь, в собственном namespace.

  Класс RequestHandler содержит путь к статическим файлам, database игры и strand/mutex

    В нём ловятся все RequestException, он производит обработку запросов к файлам. Весь код с strand оставлен закомментированным для верой в лучшее и демонстрирования того, как это было (у меня так и не получилось починить). 
    Получает лямбда-логгер, который вытягивает из ответа нужную информацию, это всё завёрнуто в transmitter

// // // automation.h // // //

Содержит функцию парсинга строки и вспомогательную структуру для возврата параметров