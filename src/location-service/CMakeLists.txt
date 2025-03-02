build_lib(
  LIBNAME location-service
  SOURCE_FILES
    model/location-service.cc
    model/god.cc
  HEADER_FILES
    model/location-service.h
    model/god.h
  LIBRARIES_TO_LINK
    ${libapplications}
    ${libinternet-apps}
    ${libwifi}
)
