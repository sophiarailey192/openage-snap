# Copyright 2013-2015 the openage authors. See copying.md for legal info.

# media files conversion stuff

from collections import defaultdict
from openage.log import dbg, ifdbg, set_verbosity
from tempfile import gettempdir
import os
import os.path
import pickle
import subprocess

from . import filelist
from . import util
from .dataformat.data_formatter import DataFormatter
from .hardcoded import termcolors
from .texture import Texture


asset_folder = ""  # TODO: optimize out


class ExtractionRule:
    """
    rule for matching media file names
    """

    def __init__(self, rulestr):
        drsname, fname = rulestr.split(':')
        fnostr, fext = fname.split('.')

        if drsname == '*':
            drsname = None

        if fnostr == '*':
            fno = None
        else:
            fno = int(fnostr)

        if fext == '*':
            fext = None

        self.drsname = drsname
        self.fno = fno
        self.fext = fext

    def matches(self, drsname, fno, fext):
        if self.drsname and self.drsname != drsname:
            return False

        if self.fno and self.fno != fno:
            return False

        if self.fext and self.fext != fext:
            return False

        return True


def media_convert(args):
    # assume to extract all files when nothing specified.
    if not args.extract:
        args.extract.append('*:*.*')

    extraction_rules = [ExtractionRule(e) for e in args.extract]
    # gamedata.drs does not exist in HD edition,
    # but its contents are in gamedata_x1.drs instead,
    # so we can ignore this file if it doesn't exist

    palette_id = None  # 50500
    palette = None  # read_palettes()

    # metadata dumping output format, more to come?
    output_formats = None  # metadata_formats

    termcolortable = None  # read_termcolortable()

    # saving files is disabled by default
    write_enabled = False

    if args.output:
        from .slp import SLP

        write_enabled = True

        dbg("setting write dir to " + args.output, 1)
        util.set_write_dir(args.output)

        player_palette = None  # read_palettes()

        # HD Edition: blendomatic_x1.dat
        # AoK:TC: blendomatic.dat

        blend_data = None  # read_blendomatic()

        stringres = None  # read_langfiles()

        gamedata = None  # read_gamedata()

        # BLOCK BORDER: gamedata / output formatting

        dbg("formatting output data...", lvl=1)
        data_formatter = DataFormatter()

        # dump metadata information
        data_dump = list()
        data_dump += blend_data.dump("blending_modes")
        data_dump += player_palette.dump("player_palette_%d" % palette_id)
        data_dump += termcolortable.dump("termcolors")
        data_dump += stringres.dump("string_resources")
        data_formatter.add_data(data_dump)

        # dump gamedata datfile data
        gamedata_dump = gamedata.dump("gamedata")
        data_formatter.add_data(gamedata_dump[0], prefix="gamedata/")

        output_data = data_formatter.export(output_formats)

        # save the meta files
        dbg("saving output data files...", lvl=1)
        util.file_write_multi(output_data, file_prefix=asset_folder)

    file_list = defaultdict(lambda: list())
    media_files_extracted = 0

    # iterate over all available files in the drs, check whether they should
    # be extracted
    for drsname, drsfile in drsfiles.items():
        for file_extension, file_id in drsfile.files:
            if not any(er.matches(drsname, file_id, file_extension)
                       for er in extraction_rules):
                continue

            # append this file to the list result
            if args.list_files:
                file_list[file_id].append((drsfile.fname, file_extension))
                continue

            # generate output filename where data will be stored in
            if write_enabled:
                fbase = os.path.join(asset_folder, drsfile.fname, str(file_id))
                fname = "%s.%s" % (fbase, file_extension)

                dbg("Extracting to %s..." % (fname), 2)
                file_data = drsfile.get_file_data(file_extension, file_id)
            else:
                continue

            if file_extension == 'slp':
                s = SLP(file_data)
                out_file_tmp = "%s: %d.%s" % (drsname, file_id, file_extension)

                dbg("%s -> %s -> generating atlas" % (out_file_tmp, fname), 1)

                # create exportable texture from the slp
                texture = Texture(s, palette)

                # the hotspots of terrain textures have to be fixed:
                if drsname == "terrain":
                    for entry in texture.image_metadata:
                        entry["cx"] = 48
                        entry["cy"] = 24

                # save the image and the corresponding metadata file
                texture.save(fname, output_formats)

            elif file_extension == 'wav':
                sound_filename = fname

                wav_output_file = util.file_get_path(fname, write=True)
                util.file_write(wav_output_file, file_data)

                if not args.no_opus:
                    file_extension = "opus"
                    sound_filename = "%s.%s" % (fbase, file_extension)
                    opus_output_file = util.file_get_path(sound_filename,
                                                          write=True)

                    # opusenc invokation (TODO: ffmpeg? some python-lib?)
                    opus_convert_call = ('opusenc',
                                         wav_output_file,
                                         opus_output_file)
                    dbg("opus convert: %s -> %s ..." % (fname,
                                                        sound_filename), 1)

                    # TODO: when the output is big enough, this deadlocks.
                    oc = subprocess.Popen(opus_convert_call,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.PIPE)
                    oc_out, oc_err = oc.communicate()

                    if ifdbg(2):
                        oc_out = oc_out.decode("utf-8")
                        oc_err = oc_err.decode("utf-8")

                        dbg(oc_out + "\n" + oc_err, 2)

                    # remove extracted original wave file
                    os.remove(wav_output_file)

            else:
                # format does not require conversion, store it as plain blob
                output_file = util.file_get_path(fname, write=True)
                util.file_write(output_file, file_data)

            media_files_extracted += 1

    if write_enabled:
        dbg("media files extracted: %d" % (media_files_extracted), 0)

    # was a file listing requested?
    if args.list_files:
        for idx, f in file_list.items():
            print("%d = [ %s ]" % (idx, ", ".join(
                "%s/%d.%s" % ((file_name, idx, file_extension)
                              for file_name, file_extension in f))))
