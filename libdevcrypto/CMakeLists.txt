file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE SRC_LIST "*.cpp")
file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.h")

# set(INSERT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/CommonFunc.cpp)
list(APPEND SRC_LIST ${INSERT_SOURCES})

add_library(devcrypto ${SRC_LIST} ${HEADERS})
eth_use(devcrypto OPTIONAL OpenSSL)
target_include_directories(devcrypto SYSTEM PRIVATE "../")
target_link_libraries(devcrypto PRIVATE Secp256k1 libff::ff libscrypt::scrypt)
target_link_libraries(devcrypto PUBLIC Secp256k1 devcore)



