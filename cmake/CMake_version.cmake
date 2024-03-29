
# Prioritize: Gitlab CI env vars CI_COMMIT_TAG and CI_COMMIT_SHORT_SHA,
# in that order, followed by sys call to git to determine commit hash or tag.

set(GIT_VERSION_STRING "")
set(CI_COMMIT_TAG_DEFINED "NO")  # YES or NO
set(CI_COMMIT_SHORT_SHA_DEFINED "NO")  # YES or NO

if(NOT "${USER_VERSION}" STREQUAL OFF)
	set(GIT_VERSION_STRING "${USER_VERSION}")	
else()

	if(NOT "${CI_COMMIT_TAG}" STREQUAL OFF)
		#message(STATUS "CI_COMMIT_TAG defined")
		if(NOT "${CI_COMMIT_TAG}" STREQUAL "")
			set(CI_COMMIT_TAG_DEFINED "YES")
			#message(STATUS "CI_COMMIT_TAG not empty string: ${CI_COMMIT_TAG}")
		endif()
	endif()

	if(NOT "${CI_COMMIT_SHORT_SHA}" STREQUAL OFF)
		#message(STATUS "CI_COMMIT_SHORT_SHA defined")
		if(NOT "${CI_COMMIT_SHORT_SHA}" STREQUAL "")
			set(CI_COMMIT_SHORT_SHA_DEFINED "YES")
			#message(STATUS "CI_COMMIT_SHORT_SHA not empty string: ${CI_COMMIT_SHORT_SHA}")
		endif()
	endif()

	if("${CI_COMMIT_TAG_DEFINED}" STREQUAL "YES")
		set(GIT_VERSION_STRING "${CI_COMMIT_TAG}")
	elseif("${CI_COMMIT_SHORT_SHA_DEFINED}" STREQUAL "YES")
		set(GIT_VERSION_STRING "${CI_COMMIT_SHORT_SHA}")
	else()

		# Initialize vars 
		set(GIT_SHORT_HASH "")
		set(GIT_SHORT_HASH_ERROR "")
		set(GIT_TAG "")
		set(GIT_TAG_ERROR "")

		# Get the latest commit hash
		execute_process(COMMAND git log --pretty=format:%h -n 1
						OUTPUT_VARIABLE GIT_SHORT_HASH
						ERROR_VARIABLE GIT_SHORT_HASH_ERROR
						OUTPUT_STRIP_TRAILING_WHITESPACE)
		#message(STATUS "git short hash: " ${GIT_SHORT_HASH})
		#message(STATUS "git short hash error: " ${GIT_SHORT_HASH_ERROR})

		if(NOT "${GIT_SHORT_HASH_ERROR}" STREQUAL "")
			message(WARNING "Neither CI_COMMIT_SHORT_SHA nor CI_COMMIT_TAG defined. Trying to read from Git")
			message(FATAL_ERROR "Error in attempt to obtain git hash: ${GIT_SHORT_HASH_ERROR}. Use -DUSER_VERSION to manually specify version.")
		endif()

		# Get the git tag for the latest commit hash if it exists.
		execute_process(COMMAND git tag -l --contains ${GIT_SHORT_HASH}
						OUTPUT_VARIABLE GIT_TAG
						ERROR_VARIABLE GIT_TAG_ERROR
						OUTPUT_STRIP_TRAILING_WHITESPACE)
		#message(STATUS "git tag: " ${GIT_TAG})
		#message(STATUS "git tag error: " ${GIT_TAG_ERROR})

		if(NOT "${GIT_TAG_ERROR}" STREQUAL "")
			message(WARNING "Neither CI_COMMIT_SHORT_SHA nor CI_COMMIT_TAG defined. Trying to read from Git")
			message(FATAL_ERROR "Error in attempt to obtain git tag: ${GIT_TAG_ERROR}. Use -DUSER_VERSION to manually specify version.")
		endif()

		# Use the tag instead of the hash if it exists
		if(NOT "${GIT_TAG}" STREQUAL "")
			set(GIT_VERSION_STRING "${GIT_TAG}")
		else()
			set(GIT_VERSION_STRING "${GIT_SHORT_HASH}")
		endif()
	endif()
endif()
