import datetime

def parsed_1553f1(obj, path):
    if isinstance(obj, str):

        if "uid" in path:

            # Handle case of the Ch10 resource for which the UID
            # ought to be the same each time.
            if "resource" in path:
                return False

            return True

        # Tip versions are not required to be equal
        elif "version" in path:
            return True
        else:
            return False

    # The "time" field is read into yaml as a datetime object.
    # Time values are not required to be equal.
    elif isinstance(obj, datetime.datetime):
        return True
    else:
        return False



def translated_1553f1(obj, path):
    if isinstance(obj, str):
        if "uid" in path:

            # Handle case of the Ch10 resource for which the UID
            # ought to be the same each time. The metadata algorithm
            # is currently configured to insert the ch10 resource
            # as the third in the "resource" sequence.
            if "['resource'][2]" in path:
                return False

            # The first resource is the DTS1553 which ought to have
            # the same uid every time.
            if "['resource'][0]" in path:
                return False

            return True

        # Tip versions are not required to be equal
        elif "version" in path:
            return True
        else:
            return False

    # The "time" field is read into yaml as a datetime object.
    # Time values are not required to be equal.
    elif isinstance(obj, datetime.datetime):
        return True
    else:
        return False

def translated_arinc429f0(obj, path):
    if isinstance(obj, str):
        if "uid" in path:

            # Handle case of the Ch10 resource for which the UID
            # ought to be the same each time. The metadata algorithm
            # is currently configured to insert the ch10 resource
            # as the third in the "resource" sequence.
            if "['resource'][2]" in path:
                return False

            # The first resource is the DTS429 which ought to have
            # the same uid every time.
            if "['resource'][0]" in path:
                return False

            return True

        # Tip versions are not required to be equal
        elif "version" in path:
            return True
        else:
            return False

    # The "time" field is read into yaml as a datetime object.
    # Time values are not required to be equal.
    elif isinstance(obj, datetime.datetime):
        return True
    else:
        return False


def parsed_videof0(obj, path):
    if isinstance(obj, str):
        if "uid" in path:

            # Handle case of the Ch10 resource for which the UID
            # ought to be the same each time.
            if "resource" in path:
                return False

            return True

        # Tip versions are not required to be equal
        elif "version" in path:
            return True
        else:
            return False

    # The "time" field is read into yaml as a datetime object.
    # Time values are not required to be equal.
    elif isinstance(obj, datetime.datetime):
        return True
    else:
        return False
