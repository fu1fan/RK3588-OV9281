"""Upload confirmation and list visibility are separate CNB operations."""
from contextlib import ExitStack
import os
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import publish


class UploadVisibilityTests(unittest.TestCase):
    def setUp(self):
        self.mocks = ExitStack()
        self.addCleanup(self.mocks.close)
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.path = Path(self.directory.name) / 'SHA256SUMS'
        self.path.write_text('verified artifact hashes\n')
        with mock.patch.dict(os.environ, CNB_TOKEN='test-only', CNB_REPO_SLUG='owner/repo'):
            self.api = publish.API()
        self.asset = {'name': self.path.name, 'size': self.path.stat().st_size,
                      'digest': publish.digest(self.path)}

    def mock_upload(self, listings):
        assets = self.mocks.enter_context(mock.patch.object(self.api, 'assets', side_effect=listings))
        request = self.mocks.enter_context(mock.patch.object(self.api, 'request', side_effect=[
            {'upload_url': 'https://storage.example/upload',
             'verify_url': 'https://api.cnb.cool/verify'}, None]))
        send = self.mocks.enter_context(mock.patch.object(publish.urllib.request, 'urlopen'))
        sleep = self.mocks.enter_context(mock.patch.object(publish.time, 'sleep'))
        return assets, request, send, sleep

    def test_immediately_visible_does_not_sleep(self):
        assets, request, send, sleep = self.mock_upload([[], [self.asset]])
        self.api.upload('/releases/123', self.path)
        sleep.assert_not_called()
        self.assertEqual(assets.call_count, 2)
        self.assertEqual(request.call_count, 2)
        send.assert_called_once()

    def test_delayed_visibility_retries_only_listing(self):
        assets, request, send, sleep = self.mock_upload([[], [], [], [self.asset]])
        self.api.upload('/releases/123', self.path)
        self.assertEqual(sleep.call_args_list, [mock.call(2), mock.call(4)])
        self.assertEqual(assets.call_count, 4)
        self.assertEqual(request.call_count, 2)
        send.assert_called_once()

    def test_last_query_can_succeed_with_commit_asset_fields(self):
        asset = {'name': self.path.name, 'size_in_byte': self.asset['size'],
                 'hash_value': self.asset['digest']}
        assets, request, send, sleep = self.mock_upload([[]] * 6 + [[asset]])
        self.api.upload('/git/commit-assets/abc', self.path)
        self.assertEqual(assets.call_count, 7)
        self.assertEqual(sleep.call_args_list, [mock.call(n) for n in (2, 4, 6, 8, 10)])
        send.assert_called_once()

    def test_missing_asset_fails_after_bounded_wait(self):
        assets, request, send, sleep = self.mock_upload([[]] * 7)
        with self.assertRaisesRegex(RuntimeError, 'not visible after 6 queries: SHA256SUMS'):
            self.api.upload('/releases/123', self.path)
        self.assertEqual(assets.call_count, 7)
        self.assertEqual(sum(call.args[0] for call in sleep.call_args_list), 30)
        self.assertEqual(request.call_count, 2)
        send.assert_called_once()

    def test_visible_wrong_hash_is_not_accepted_or_retried(self):
        assets, request, send, sleep = self.mock_upload([[], [], [{**self.asset, 'digest': 'wrong'}]])
        with self.assertRaisesRegex(RuntimeError, 'Remote asset checksum mismatch'):
            self.api.upload('/releases/123', self.path)
        self.assertEqual(assets.call_count, 3)
        sleep.assert_called_once_with(2)
        send.assert_called_once()

    def test_visible_wrong_size_is_not_accepted(self):
        assets, request, send, sleep = self.mock_upload([[], [{**self.asset, 'size': 0}]])
        with self.assertRaisesRegex(RuntimeError, 'Remote asset checksum mismatch'):
            self.api.upload('/releases/123', self.path)
        sleep.assert_not_called()

    def test_query_error_is_not_hidden_as_eventual_consistency(self):
        assets, request, send, sleep = self.mock_upload([[], publish.APIError(403)])
        with self.assertRaises(publish.APIError) as error:
            self.api.upload('/releases/123', self.path)
        self.assertEqual(error.exception.code, 403)
        sleep.assert_not_called()


if __name__ == '__main__':
    unittest.main()
