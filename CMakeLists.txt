cmake_minimum_required(VERSION 3.17)
project(SuccinctEdge_cata)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -latomic")

include_directories(usr/include /usr/local/lib/ ./lib/sdsl-lite-2.1.1/include)
link_directories(/usr/lib ./lib/sdsl-lite-2.1.1/lib)
link_libraries(sdsl divsufsort divsufsort64 libmosquitto.so libparquet.so libarrow.so)
add_executable(SuccinctEdge_cata main.cpp TripleStore.cpp TripleStore.hpp RDFStorage.cpp RDFStorage.hpp Dictionary.cpp Dictionary.hpp RankBaseElement.cpp RankBaseElement.hpp Query.cpp Query.h Util.hpp RDFTypeStore.cpp RDFTypeStore.hpp Constant.hpp quickSortTS.hpp TriplePattern.cpp TriplePattern.hpp LiteMatDictionary.cpp LiteMatDictionary.hpp JoinLine.cpp JoinLine.hpp JoinVariables.cpp JoinVariables.hpp JoinTable.cpp JoinTable.hpp QueryEngine.cpp QueryEngine.hpp BitMap.cpp BitMap.hpp DataTypeStore.cpp DataTypeStore.hpp PropertyLiteMatDictionary.cpp PropertyLiteMatDictionary.hpp StrDataRankBaseElement.cpp StrDataRankBaseElement.hpp DataSeries.cpp DataSeries.hpp DataBlock.cpp DataBlock.hpp Filter.hpp Filter.cpp TCPReceiver.cpp TCPReceiver.hpp ShowResults.hpp DataWindowUtil.hpp MiniBatchUtil.hpp TimeUtil.hpp)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")